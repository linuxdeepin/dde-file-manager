// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexprofile.h"
#include "extractor/contentdeduplication.h"
#include "extractor/ocrdeduplication.h"
#include "lowercasengramanalyzer.h"
#include "utils/filehash.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"

#include <dfm-search/field_names.h>
#include <lucene++/LuceneHeaders.h>

#include <QDir>
#include <QFileInfo>

SERVICETEXTINDEX_BEGIN_NAMESPACE

IndexProfile::Type IndexProfile::type() const
{
    return m_identity.type;
}

const QString &IndexProfile::id() const
{
    return m_identity.id;
}

QString IndexProfile::indexDirectory() const
{
    return m_providers.indexDirectory ? m_providers.indexDirectory() : QString();
}

QString IndexProfile::statusFilePath() const
{
    const QString indexDir = indexDirectory();
    if (indexDir.isEmpty()) {
        return QString();
    }

    return indexDir + QLatin1Char('/') + m_identity.statusFileName;
}

const QString &IndexProfile::versionKey() const
{
    return m_identity.versionKey;
}

bool IndexProfile::isIndexAvailable() const
{
    return m_providers.availability ? m_providers.availability() : false;
}

int IndexProfile::runtimeIndexVersion() const
{
    return m_identity.runtimeVersion;
}

bool IndexProfile::isPathInScope(const QString &path) const
{
    return m_providers.scope ? m_providers.scope(path) : false;
}

bool IndexProfile::isCandidateFile(const QString &path) const
{
    return m_providers.candidate ? m_providers.candidate(path) : false;
}

IndexProfile::AnythingSearchOptions IndexProfile::anythingSearchOptions() const
{
    return m_providers.anythingSearchOptions ? m_providers.anythingSearchOptions() : AnythingSearchOptions {};
}

bool IndexProfile::supportsAnything() const
{
    return !anythingSearchOptions().isEmpty();
}

QString IndexProfile::computeChecksum(const QString &filePath) const
{
    return m_providers.checksum ? m_providers.checksum(filePath) : QString();
}

QString IndexProfile::lookupCachedText(const QString &checksum) const
{
    return m_providers.textCache ? m_providers.textCache(checksum) : QString();
}

bool IndexProfile::supportsChecksum() const
{
    return static_cast<bool>(m_providers.checksum);
}

bool IndexProfile::shouldCheckEnvPolicy() const
{
    return m_runtimePolicy.checkEnvPolicy;
}

const IndexProfile::RuntimePolicy &IndexProfile::runtimePolicy() const
{
    return m_runtimePolicy;
}

const IndexProfile::FilterPolicy &IndexProfile::filterPolicy() const
{
    return m_filterPolicy;
}

bool IndexProfile::shouldSkipHiddenEntry(const QString &path) const
{
    // 只对隐藏路径有意义；非隐藏路径恒不跳过
    if (!DFMSEARCH::Global::isHiddenPathOrInHiddenDir(path))
        return false;

    // Content/Ocr：完全不索引隐藏条目（原有行为）
    if (!m_filterPolicy.indexHiddenFiles)
        return true;

    // 二级策略（filename）：屏蔽策略命中的隐藏条目（默认主目录下）
    return m_filterPolicy.hiddenIndexExclusion
            && m_filterPolicy.hiddenIndexExclusion(path);
}

IndexProfile::MoveUpdatePolicy IndexProfile::moveUpdatePolicy() const
{
    return m_capabilities.moveUpdatePolicy;
}

bool IndexProfile::requiresContentExtraction() const
{
    return m_capabilities.requiresContentExtraction;
}

int IndexProfile::lightGradeFileCountThreshold() const
{
    return m_capabilities.lightGradeCountThreshold ? m_capabilities.lightGradeCountThreshold() : 0;
}

const wchar_t *IndexProfile::pathField() const
{
    switch (m_identity.type) {
    case Type::Ocr:
        return DFMSEARCH::LuceneFieldNames::OcrText::kPath;
    case Type::Filename:
        return DFMSEARCH::LuceneFieldNames::FileName::kFullPath;
    case Type::Content:
    default:
        return DFMSEARCH::LuceneFieldNames::Content::kPath;
    }
}

const wchar_t *IndexProfile::contentField() const
{
    switch (m_identity.type) {
    case Type::Ocr:
        return DFMSEARCH::LuceneFieldNames::OcrText::kOcrContents;
    case Type::Filename:
        return DFMSEARCH::LuceneFieldNames::FileName::kFileName;
    case Type::Content:
    default:
        return DFMSEARCH::LuceneFieldNames::Content::kContents;
    }
}

const wchar_t *IndexProfile::ancestorPathsField() const
{
    switch (m_identity.type) {
    case Type::Ocr:
        return DFMSEARCH::LuceneFieldNames::OcrText::kAncestorPaths;
    case Type::Filename:
        return DFMSEARCH::LuceneFieldNames::FileName::kAncestorPaths;
    case Type::Content:
    default:
        return DFMSEARCH::LuceneFieldNames::Content::kAncestorPaths;
    }
}

const wchar_t *IndexProfile::modifyTimeField() const
{
    switch (m_identity.type) {
    case Type::Ocr:
        return DFMSEARCH::LuceneFieldNames::OcrText::kModifyTime;
    case Type::Filename:
        return DFMSEARCH::LuceneFieldNames::FileName::kModifyTime;
    case Type::Content:
    default:
        return DFMSEARCH::LuceneFieldNames::Content::kModifyTime;
    }
}

bool IndexProfile::supportsModifiedTimestampCheck() const
{
    switch (m_identity.type) {
    case Type::Ocr:
    case Type::Content:
    case Type::Filename:
        return true;
    default:
        return false;
    }
}

boost::shared_ptr<void> IndexProfile::createAnalyzer() const
{
    return m_providers.analyzer ? m_providers.analyzer() : nullptr;
}

int IndexProfile::maxFileTruncationSizeMB() const
{
    const TextIndexConfig &config = TextIndexConfig::instance();
    switch (m_identity.type) {
    case Type::Ocr:
        return config.maxOcrImageSizeMB();
    case Type::Filename:
        return config.maxIndexFileTruncationSizeMB();
    case Type::Content:
    default:
        return config.maxIndexFileTruncationSizeMB();
    }
}

IndexProfile IndexProfile::content()
{
    // Capture content index directory for use in the text cache lookup lambda.
    const QString contentIndexDir = DFMSEARCH::Global::contentIndexDirectory();

    Providers providers;
    providers.indexDirectory = []() { return DFMSEARCH::Global::contentIndexDirectory(); };
    providers.availability = []() { return DFMSEARCH::Global::isContentIndexAvailable(); };
    providers.scope = [](const QString &path) { return DFMSEARCH::Global::isPathInContentIndexDirectory(path); };
    providers.candidate = [](const QString &path) { return IndexUtility::isSupportedTextFile(path); };
    providers.anythingSearchOptions = []() {
        return AnythingSearchOptions {
            { Defines::kAnythingDocType },
            TextIndexConfig::instance().supportedTextFileExtensions()
        };
    };
    // ChecksumProvider: compute MD5 only for files larger than 1MB
    providers.checksum = [](const QString &filePath) {
        constexpr qint64 kMinFileSizeForChecksum = 1LL * 1024 * 1024;   // 1MB
        QFileInfo fi(filePath);
        if (fi.size() <= kMinFileSizeForChecksum) {
            return QString();
        }
        return FileHash::computeMd5(filePath);
    };
    // TextCacheLookup: find existing content text by checksum
    providers.textCache = [contentIndexDir](const QString &checksum) {
        return ContentDeduplication::lookupByTextChecksum(checksum, contentIndexDir);
    };
    providers.analyzer = []() -> boost::shared_ptr<void> {
        return Lucene::newLucene<LowerCaseNGramAnalyzer>(1, 2);
    };

    return IndexProfile {
        Identity { Type::Content, QStringLiteral("content"),
                   QStringLiteral("index_status.json"), Defines::kTextVersionKey, Defines::kTextIndexVersion },
        std::move(providers)
    };
}

IndexProfile IndexProfile::ocr()
{
    // Capture OCR index directory for use in the text cache lookup lambda.
    // This avoids repeatedly querying the global function on every lookup call.
    const QString ocrIndexDir = DFMSEARCH::Global::ocrTextIndexDirectory();

    Providers providers;
    providers.indexDirectory = []() { return DFMSEARCH::Global::ocrTextIndexDirectory(); };
    providers.availability = []() { return DFMSEARCH::Global::isOcrTextIndexAvailable(); };
    providers.scope = [](const QString &path) { return DFMSEARCH::Global::isPathInOcrTextIndexDirectory(path); };
    providers.candidate = [](const QString &path) { return IndexUtility::isSupportedOCRFile(path); };
    providers.anythingSearchOptions = []() {
        return AnythingSearchOptions {
            { Defines::kAnythingPicType },
            TextIndexConfig::instance().supportedOcrImageExtensions()
        };
    };
    // ChecksumProvider: compute file MD5
    providers.checksum = [](const QString &filePath) { return FileHash::computeMd5(filePath); };
    // TextCacheLookup: find existing OCR text by checksum
    providers.textCache = [ocrIndexDir](const QString &checksum) {
        return OcrDeduplication::lookupByTextChecksum(checksum, ocrIndexDir);
    };
    // AnalyzerProvider: create lowercase NGram analyzer for OCR text
    providers.analyzer = []() -> boost::shared_ptr<void> {
        return Lucene::newLucene<LowerCaseNGramAnalyzer>(1, 2);
    };

    Capabilities capabilities;
    // OCR 单文件提取成本高，Light 分级阈值独立于全局配置（保持 dconfig 动态性）
    capabilities.lightGradeCountThreshold = []() {
        return TextIndexConfig::instance().lightIncrementOcrFileCountThreshold();
    };

    return IndexProfile {
        Identity { Type::Ocr, QStringLiteral("ocr"),
                   QStringLiteral("index_status.json"), Defines::kOcrVersionKey, Defines::kOcrIndexVersion },
        std::move(providers),
        RuntimePolicy{},
        FilterPolicy{},
        std::move(capabilities)
    };
}

IndexProfile IndexProfile::filename()
{
    FilterPolicy filterPolicy;
    filterPolicy.indexHiddenFiles = true;
    // 隐藏文件二级策略：filename 虽索引隐藏文件（标记 is_hidden 字段），但主目录
    // 第一级的隐藏条目（~/.local、~/.cache 等，数量巨大）默认屏蔽，控制索引规模、
    // 构建时长与 inotify 事件量。更深层级的隐藏目录（如 ~/Documents/.abc）依然
    // 索引。搜索端（dfm-plugin-search）在"显示隐藏文件"开启且搜索主目录（或其
    // 祖先）时回退实时搜索兜底，保证该场景下第一级隐藏条目仍可被搜到。
    filterPolicy.hiddenIndexExclusion = [](const QString &hiddenPath) {
        static const QString home = QDir::homePath();
        // 仅屏蔽主目录第一级隐藏条目（~/.local 等，其整个子树随之排除）；
        // 更深层级的隐藏条目（~/Documents/.abc）不属于第一级，依然索引
        if (!hiddenPath.startsWith(home + QLatin1Char('/')))
            return false;
        const int firstSegmentEnd = hiddenPath.indexOf(QLatin1Char('/'), home.size() + 1);
        const QString firstSegment = firstSegmentEnd < 0
                ? hiddenPath.mid(home.size() + 1)
                : hiddenPath.mid(home.size() + 1, firstSegmentEnd - home.size() - 1);
        return firstSegment.startsWith(QLatin1Char('.'));
    };
    // 目录自身入索引（file_type=dir）：文件管理器支持按名称搜索目录，
    // 文档字段（file_name/pinyin/file_type 等）全部由路径派生，与文件同构
    filterPolicy.indexDirectories = true;
    // link 条目本身入索引：链接文件/dir 链接按自身路径可搜（不跟随、不递归）。
    // 文档全部由路径派生，对 dangling link 也成立（birth/size 为缺省值）。
    filterPolicy.indexSymlinks = true;
    // 黑名单 = anything blacklist_paths + 三个索引目录 + .avfs（死循环防护）：
    // filename 自身是索引提供者，索引目录位于 $HOME 下，若不排除，
    // 索引器写入 Lucene 段文件会产生新事件 → 再次索引 → 无限循环。
    filterPolicy.blacklistProvider = []() -> QStringList {
        QStringList blacklist = IndexUtility::AnythingConfigWatcher::instance()->defaultBlacklistPaths();
        blacklist << DFMSEARCH::Global::fileNameIndexDirectory()
                  << DFMSEARCH::Global::contentIndexDirectory()
                  << DFMSEARCH::Global::ocrTextIndexDirectory()
                  << QStringLiteral(".avfs");
        return blacklist;
    };

    // 恢复静默延迟 3s（Content/Ocr 为全局 180s）：filename 恢复 Update 期间
    // updateInProgress=true 持续降级搜索，缩短延迟可尽快回到索引搜索；
    // filename 构建只读文件元数据（无内容提取），提前扫盘 IO 风险低。
    RuntimePolicy runtimePolicy;
    runtimePolicy.checkEnvPolicy = false;   // 环境策略豁免（电池/节能/空闲）
    runtimePolicy.eventCollectionWindowMs = 1000;   // 1s 收集窗口，复用原有 commit 机制
    runtimePolicy.recoveryUpdateDelayMs = 3000;   // 恢复 Update 静默延迟 3s

    Capabilities capabilities;
    // 文档业务字段（file_name/pinyin/file_ext/file_type/is_hidden 等）全部由路径派生，
    // move/rename 后必须整体重建文档，仅更新 path 会残留旧文件名的索引值
    capabilities.moveUpdatePolicy = MoveUpdatePolicy::RebuildDocument;
    // filename 只读文件元数据，无内容提取
    capabilities.requiresContentExtraction = false;

    Providers providers;
    providers.indexDirectory = []() { return DFMSEARCH::Global::fileNameIndexDirectory(); };
    providers.availability = []() { return DFMSEARCH::Global::isFileNameIndexDirectoryAvailable(); };
    providers.scope = [](const QString &path) { return DFMSEARCH::Global::isPathInFileNameIndexDirectory(path); };
    providers.candidate = [](const QString &) { return true; };
    // no anything search options → FileSystemProvider 全盘遍历；no checksum/text cache
    providers.analyzer = []() -> boost::shared_ptr<void> {
        return Lucene::newLucene<LowerCaseNGramAnalyzer>(1, 2);
    };

    return IndexProfile {
        Identity { Type::Filename, QStringLiteral("filename"),
                   QStringLiteral("index_status.json"), Defines::kFilenameVersionKey, Defines::kFilenameIndexVersion },
        std::move(providers),
        std::move(runtimePolicy),
        std::move(filterPolicy),
        std::move(capabilities)
    };
}

SERVICETEXTINDEX_END_NAMESPACE
