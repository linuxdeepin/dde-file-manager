// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXPROFILE_H
#define INDEXPROFILE_H

#include "service_textindex_global.h"

#include <boost/shared_ptr.hpp>

#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexProfile
{
public:
    struct AnythingSearchOptions {
        QStringList fileTypes;
        QStringList fileExtensions;

        bool isEmpty() const
        {
            return fileTypes.isEmpty() && fileExtensions.isEmpty();
        }
    };

    enum class Type {
        Content,
        Ocr,
        Filename
    };

    /// 运行时调度策略（环境豁免/事件窗口/恢复延迟）
    struct RuntimePolicy {
        bool checkEnvPolicy { true };
        int eventCollectionWindowMs { 3000 };
        // dirty 重启后恢复 Update 的静默延迟（毫秒），0 表示用全局 silentIndexUpdateDelay
        // Content/Ocr: 0（全局默认 180s）
        // Filename: 30000（30s，缩短 updateInProgress 降级窗口）
        int recoveryUpdateDelayMs { 0 };
    };

    /// 过滤策略（隐藏文件/黑名单/目录索引）
    struct FilterPolicy {
        bool indexHiddenFiles { false };
        // 隐藏文件二级策略（indexHiddenFiles 为 true 时有意义）：
        // 给定隐藏条目的完整路径，返回 true 表示该条目不索引（其子树同样不遍历，
        // 对应的文件系统事件照旧丢弃）。为空时隐藏条目全部索引。
        // Filename: 仅屏蔽主目录第一级隐藏条目（~/.local、~/.bashrc 等巨量
        // 缓存/配置/应用数据，会显著放大索引规模、构建时长与 inotify 事件量）；
        // 更深层级中的隐藏目录（如 ~/Documents/.abc）依然索引。搜索端
        // （dfm-plugin-search）在"显示隐藏文件"开启且搜索主目录（或其祖先）
        // 时回退实时搜索兜底，保证该场景下第一级隐藏条目仍可被搜到。
        // Content/Ocr: 无（indexHiddenFiles=false 时隐藏条目本来就全部跳过）。
        std::function<bool(const QString &hiddenPath)> hiddenIndexExclusion;
        // 是否将目录自身作为文档写入索引（file_type=dir）。
        // Filename: true（支持按名称搜索目录）；
        // Content/Ocr: false（目录无内容可提取，仅作遍历容器不建档）
        bool indexDirectories { false };
        // 符号链接策略：link 条目本身是否入索引（按链接自身路径/名称建档，
        // 绝不跟随——不递归进 dir link、不监控 link 内部，防索引环）。
        // Filename: true（链接文件/目录链接在文件管理器中是真实可见条目，
        // 按名称搜索应能命中；link 内部内容不索引，仅链接名可搜）；
        // Content/Ocr: false（行为不变，symlink 事件/条目照旧被排除）。
        bool indexSymlinks { false };
        // 黑名单路径提供器（函数式注入）；为空时使用全局 createForIndex() 匹配器
        // Content/Ocr: 空（TextIndexConfig folderExcludeFilters + anything blacklist）
        // Filename: anything blacklist_paths + 三个索引目录 + .avfs（死循环防护）
        std::function<QStringList()> blacklistProvider;
    };

    /**
     * @brief 文件移动/重命名时文档的更新策略
     *
     * 由 profile 声明其文档字段对路径的依赖语义，task 层（MoveFileListHandler/
     * FileMoveProcessor）仅依赖此抽象，不得感知具体 profile 类型：
     * - UpdatePathOnly: 保留文档中与路径无关的字段（contents/checksum/时间戳等），
     *   仅更新 path/ancestor_paths 及 builder 声明的路径派生字段
     *   （见 IndexDocumentBuilder::pathDerivedFields），避免重新提取内容
     * - RebuildDocument: 文档业务字段全部由路径派生（如 Filename 的 file_name/
     *   pinyin/file_ext/file_type/is_hidden），文件改名后必须整体重建文档，
     *   否则旧路径派生的字段值残留导致新名称搜索不到。
     *   仅作用于文件级 move（FileMoveProcessor）；目录级 move 不改变子文件
     *   的 basename，统一走 copy + 派生字段重算（DirectoryMoveProcessor）
     */
    enum class MoveUpdatePolicy {
        UpdatePathOnly,
        RebuildDocument
    };

    using DirectoryProvider = std::function<QString()>;
    using AvailabilityChecker = std::function<bool()>;
    using ScopeChecker = std::function<bool(const QString &)>;
    using CandidateChecker = std::function<bool(const QString &)>;
    using AnythingSearchOptionsProvider = std::function<AnythingSearchOptions()>;
    using ChecksumProvider = std::function<QString(const QString &filePath)>;
    using TextCacheLookup = std::function<QString(const QString &checksum)>;
    using AnalyzerProvider = std::function<boost::shared_ptr<void>()>;
    using LightGradeCountThresholdProvider = std::function<int()>;

    /// 静态身份描述（不变的事实性数据）
    struct Identity {
        Type type { Type::Content };
        QString id;
        QString statusFileName { QStringLiteral("index_status.json") };
        QString versionKey;
        int runtimeVersion { -1 };
    };

    /// 数据与行为的外部提供者（函数式注入）
    struct Providers {
        DirectoryProvider indexDirectory;
        AvailabilityChecker availability;
        ScopeChecker scope;
        CandidateChecker candidate;
        AnythingSearchOptionsProvider anythingSearchOptions;
        ChecksumProvider checksum;
        TextCacheLookup textCache;
        AnalyzerProvider analyzer;
    };

    /// 能力与语义声明（文档/索引行为的自描述）
    struct Capabilities {
        // 文件移动/重命名时文档的更新策略
        MoveUpdatePolicy moveUpdatePolicy { MoveUpdatePolicy::UpdatePathOnly };
        // 是否需要内容提取器；false 时 runtime 不装配 extractor
        //（如 filename 索引只读文件元数据）
        bool requiresContentExtraction { true };
        // Light 分级文件数阈值提供器（保持 dconfig 动态性）；
        // 为空时使用全局 TextIndexConfig::lightIncrementFileCountThreshold()
        LightGradeCountThresholdProvider lightGradeCountThreshold;
    };

    IndexProfile() = default;
    IndexProfile(Identity identity, Providers providers,
                 RuntimePolicy runtimePolicy,
                 FilterPolicy filterPolicy,
                 Capabilities capabilities);

    Type type() const;
    const QString &id() const;
    QString indexDirectory() const;
    QString statusFilePath() const;
    const QString &versionKey() const;
    bool isIndexAvailable() const;
    int runtimeIndexVersion() const;
    bool isPathInScope(const QString &path) const;
    bool isCandidateFile(const QString &path) const;
    AnythingSearchOptions anythingSearchOptions() const;
    bool supportsAnything() const;

    /**
     * @brief Compute a checksum for the given file, if the profile supports it
     * @return Checksum string, or empty if the profile does not support checksumming
     */
    QString computeChecksum(const QString &filePath) const;

    /**
     * @brief Look up cached extraction text by checksum, if the profile supports it
     * @return Cached text, or empty if no cache hit or not supported
     */
    QString lookupCachedText(const QString &checksum) const;

    bool supportsChecksum() const;

    bool shouldCheckEnvPolicy() const;
    const RuntimePolicy &runtimePolicy() const;
    const FilterPolicy &filterPolicy() const;

    /**
     * @brief 隐藏条目是否应被跳过（不索引、不遍历、事件丢弃）
     *
     * 三类调用方共用此抽象：全盘遍历（FileSystemProvider）、增量路径列表
     * （MixedPathListProvider）与文件系统事件谓词（FSEventController），
     * 保证三者对"哪些隐藏条目不进索引"的判定严格一致。
     * @param path 隐藏条目的完整路径（非隐藏路径恒返回 false）
     */
    bool shouldSkipHiddenEntry(const QString &path) const;

    /**
     * @brief 文件移动/重命名时该 profile 文档的更新策略
     * @see MoveUpdatePolicy
     */

    /**
     * @brief 文件移动/重命名时该 profile 文档的更新策略
     * @see MoveUpdatePolicy
     */
    MoveUpdatePolicy moveUpdatePolicy() const;

    /**
     * @brief 该 profile 是否需要内容提取器
     *
     * false 时 runtime 不装配 extractor（如 filename 索引只读文件元数据），
     * createFileDocument/processContentUpdate 将以空文本构建文档。
     */
    bool requiresContentExtraction() const;

    /**
     * @brief Light 分级任务的最大文件数阈值
     * @return profile 声明的阈值；未声明（provider 为空）时返回 0，
     *         调用方应回退到全局 TextIndexConfig::lightIncrementFileCountThreshold()
     */
    int lightGradeFileCountThreshold() const;

    const wchar_t *pathField() const;
    const wchar_t *contentField() const;
    const wchar_t *ancestorPathsField() const;
    const wchar_t *modifyTimeField() const;
    bool supportsModifiedTimestampCheck() const;
    boost::shared_ptr<void> createAnalyzer() const;
    int maxFileTruncationSizeMB() const;

    static IndexProfile content();
    static IndexProfile ocr();
    static IndexProfile filename();

private:
    Identity m_identity;
    Providers m_providers;
    Capabilities m_capabilities;
    RuntimePolicy m_runtimePolicy;
    FilterPolicy m_filterPolicy;
};

// 定义置于类外：默认参数需要 Capabilities 的 NSDMI（嵌套类完成于外围类之后），
// 聚合初始化特性也依赖 Capabilities 保持聚合类型
inline IndexProfile::IndexProfile(Identity identity, Providers providers,
                                  RuntimePolicy runtimePolicy = RuntimePolicy{},
                                  FilterPolicy filterPolicy = FilterPolicy{},
                                  Capabilities capabilities = Capabilities{})
    : m_identity(std::move(identity)),
      m_providers(std::move(providers)),
      m_capabilities(std::move(capabilities)),
      m_runtimePolicy(std::move(runtimePolicy)),
      m_filterPolicy(std::move(filterPolicy))
{
}

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXPROFILE_H
