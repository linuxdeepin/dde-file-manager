// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXSTATESTORE_H
#define INDEXSTATESTORE_H

#include "profile/indexprofile.h"
#include "utils/indexutility.h"

#include <QAtomicInteger>
#include <QDateTime>
#include <QMutex>
#include <QStringList>

SERVICETEXTINDEX_BEGIN_NAMESPACE

/**
 * @brief 索引状态持久化存储
 *
 * IndexStateStore 负责管理索引的状态文件（一个 JSON 文件），记录索引的生命周期状态。
 * 它是全文检索服务（TextIndex / OcrIndex）的"状态大脑"，所有关于索引是否可用、
 * 是否需要重建、上次更新时间等信息都由本类持久化到磁盘。
 *
 * ## 状态文件结构
 *
 * 状态文件路径由 IndexProfile::statusFilePath() 提供，默认文件名为 index_status.json。
 * JSON 示例：
 * \code
 * {
 *     "state": "clean",            // 或 "dirty"
 *     "needsRebuild": false,       // 是否需要重建
 *     "lastUpdateTime": "2026-08-03T10:30:00",  // ISO 时间
 *     "version": 6                 // 索引版本号
 * }
 * \endcode
 *
 * ## 核心概念详解
 *
 * ### 1. state（Clean / Dirty / Unknown）—— 索引完整性标记
 *
 * - **Clean**（"clean"）：索引完整可用。上次服务正常关闭，没有未完成的任务。
 * - **Dirty**（"dirty"）：索引可能不完整。服务在上次运行时有任务未完成（如创建/更新被中断），
 *   或服务以非正常方式退出。下次启动时需要进行完整的 Update 任务来修复。
 * - **Unknown**：状态文件不存在或 state 字段缺失（旧版遗留文件或文件损坏）。
 *
 * **生命周期**：
 * - 任务启动时（startTask），state 被设为 Dirty
 * - 全量任务（Create/Update）成功完成时，state 被设为 Clean
 * - 增量任务（文件列表增删改）成功完成时，state 是否设为 Clean 取决于 recoveryPending
 * - 服务 cleanup 时若有未完成任务，state 被设为 Dirty
 *
 * ### 2. needsRebuild —— 配置变更重建标记
 *
 * 这是一个独立的布尔标记，与 state 无关。当外部配置发生变化（如 ANYTHING 索引路径变更、
 * DLNFS 挂载配置变更），ConfigWatcher 会将 needsRebuild 设为 true。
 *
 * **使用场景**：服务启动时若发现 needsRebuild=true，即使 state=Clean 也会强制启动
 * 一个 Update 任务，以确保索引内容与最新配置一致。任务启动后清除该标记。
 *
 * ### 3. RecoveryPending（本类不存储，但与 state 密切相关）—— 恢复挂起标志
 *
 * \warning RecoveryPending 不存储在状态文件中，而是 TaskManager 的内存标志（m_recoveryPending）。
 * 此处说明它是因为理解 state 的行为离不开它。
 *
 * **设计目的**：防止服务启动后，一个小的增量任务完成时把 Dirty 状态清除为 Clean，
 * 导致真正需要的完整 Update 任务无法触发。
 *
 * **场景示例**：
 * 1. 服务上次异常退出，state=Dirty
 * 2. 服务重启，initialize() 检测到 Dirty，设置 m_recoveryPending=true
 * 3. 此时文件系统监控（FSEventController）检测到文件变化，触发了一个小的增量更新任务
 * 4. 增量任务完成 → finalizeIndexState() 检查 m_recoveryPending == true → **不设 Clean**，保持 Dirty
 * 5. 同时，handleSlientStart() 检测到 recoveryPending，启动完整 Update 任务
 * 6. 完整 Update 任务成功 → m_recoveryPending 被清除为 false → state 设为 Clean
 *
 * ### 4. version + lastUpdateTime —— 版本与时间管理
 *
 * - **version**：索引版本号。当索引 schema 变化时（如新增字段），通过提升 Defines::kTextIndexVersion
 *   强制旧索引重建。isCompatibleVersion() 比较存储版本与运行时版本。
 * - **lastUpdateTime**：上次成功更新时间，供外部查询索引新鲜度。
 *
 * **关键区别**：
 * - 全量任务（Create/Update）成功 → 调用 saveIndexStatus()，同时更新 version 和 lastUpdateTime
 * - 增量任务成功 → 调用 saveLastUpdateTime()，只更新 lastUpdateTime，不改变 version
 *
 * 这样设计是为了避免增量任务意外提升 version，导致与正在挂起的恢复任务产生版本冲突。
 *
 * ## 任务类型与状态的关系
 *
 * | 任务类型 | isFullScan | 成功后行为 |
 * |---------|-----------|-----------|
 * | Create | 是 | saveIndexStatus(版本+时间), recoveryPending=false, state=Clean |
 * | Update | 是 | saveIndexStatus(版本+时间), recoveryPending=false, state=Clean |
 * | CreateFileList | 否 | saveLastUpdateTime(仅时间), state=Clean(若非recoveryPending) |
 * | UpdateFileList | 否 | saveLastUpdateTime(仅时间), state=Clean(若非recoveryPending) |
 * | RemoveFileList | 否 | saveLastUpdateTime(仅时间), state=Clean(若非recoveryPending) |
 * | MoveFileList | 否 | saveLastUpdateTime(仅时间), state=Clean(若非recoveryPending) |
 *
 * @see TaskManager::finalizeIndexState() 状态转换的核心逻辑
 * @see TextIndexDBusPrivate::handleSlientStart() 启动恢复逻辑
 */
class IndexStateStore
{
public:
    explicit IndexStateStore(IndexProfile profile);

    /**
     * @brief 返回状态文件的完整路径
     * @return 状态文件路径，形如 /path/to/index_dir/index_status.json
     */
    QString statusFilePath() const;

    /**
     * @brief 读取当前索引状态
     * @return Clean / Dirty / Unknown（文件不存在或字段无效时返回 Unknown）
     *
     * 从 JSON 的 "state" 字段读取，值为 "clean" 或 "dirty"。
     */
    IndexUtility::IndexState getIndexState() const;

    /**
     * @brief 设置索引状态并持久化
     * @param state 目标状态（仅支持 Clean 和 Dirty，Unknown 会被忽略）
     *
     * 采用 read-modify-write 模式：先读取整个 JSON，更新 state 字段，再写回。
     * 这样不会破坏文件中的其他字段（如 needsRebuild、version 等）。
     */
    void setIndexState(IndexUtility::IndexState state) const;

    /**
     * @brief 快速判断索引是否处于 Clean 状态
     * @return true 当且仅当 getIndexState() == Clean
     */
    bool isCleanState() const;

    /**
     * @brief 读取 needsRebuild 标记
     * @return true 表示配置已变更、索引需要重建；false 表示无重建需求
     *
     * 从 JSON 的 "needsRebuild" 字段读取，字段缺失时默认返回 false。
     */
    bool needsRebuild() const;

    /**
     * @brief 设置 needsRebuild 标记并持久化
     * @param need 是否需要重建
     *
     * 通常由 ConfigWatcher 的信号触发（ANYTHING/DLNFS 配置变更），
     * 也可在 handleSlientStart 中主动清除（启动 Update 任务前）。
     */
    void setNeedsRebuild(bool need) const;

    /**
     * @brief 读取 createInProgress 标记
     * @return true 表示 CREATE 任务未完成（重型 UPDATE 的判定依据）；false 表示无 CREATE 进行中
     *
     * 状态机：
     * - CREATE startTask 时置 true
     * - Create/Update 全量任务成功完成时置 false
     * - 中断/失败时不操作（保持原值），下次重启仍判定为重型
     * - 增量任务不操作
     *
     * 字段缺失时默认返回 false（旧版遗留文件或文件不存在）。
     */
    bool isCreateInProgress() const;

    /**
     * @brief 设置 createInProgress 标记并持久化
     * @param inProgress CREATE 任务是否未完成
     *
     * 采用 read-modify-write 模式：先读取整个 JSON，更新字段，再写回。
     * 不破坏文件中的其他字段（如 state、needsRebuild、version 等）。
     */
    void setCreateInProgress(bool inProgress) const;

    /**
     * @brief 获取上次更新时间
     * @return 格式化时间字符串 "yyyy-MM-dd hh:mm:ss"，文件不存在时返回空字符串
     */
    QString getLastUpdateTime() const;

    /**
     * @brief 获取索引版本号
     * @return 版本号，文件不存在或字段缺失时返回 -1
     *
     * versionKey 由 IndexProfile 提供（Content 和 OCR 各自不同的 key）。
     */
    int getIndexVersion() const;

    /**
     * @brief 检查存储的版本号是否与当前运行时版本兼容
     * @return true 表示版本匹配，索引可用；false 表示版本不匹配需重建
     *
     * 比较 getIndexVersion() 与 IndexProfile::runtimeIndexVersion()。
     * 当索引 schema 变化时，提升 kTextIndexVersion 会使旧索引自动判定为不兼容。
     */
    bool isCompatibleVersion() const;

    /**
     * @brief 删除状态文件
     *
     * 在以下场景使用：
     * - Create 任务启动时（清除旧状态）
     * - Update 任务失败且根路径在范围内时（标记索引无效）
     */
    void removeIndexStatusFile() const;

    /**
     * @brief 清空索引目录下的所有文件
     *
     * 当索引损坏（isIndexCorrupted）导致 Update 失败时，清空索引目录后重新 Create。
     */
    void clearIndexDirectory() const;

    /**
     * @brief 保存索引状态（全量任务专用，同时更新 version 和 lastUpdateTime）
     * @param lastUpdateTime 上次更新时间
     *
     * 使用 IndexProfile::runtimeIndexVersion() 作为版本号。
     * 适用于 Create / Update 全量扫描任务成功完成后。
     */
    void saveIndexStatus(const QDateTime &lastUpdateTime) const;

    /**
     * @brief 保存索引状态（指定版本号）
     * @param lastUpdateTime 上次更新时间
     * @param version 索引版本号
     *
     * read-modify-write：只更新 lastUpdateTime 和 version 字段，保留其他字段。
     */
    void saveIndexStatus(const QDateTime &lastUpdateTime, int version) const;

    /**
     * @brief 仅保存更新时间，不改变版本号（增量任务专用）
     * @param lastUpdateTime 上次更新时间
     *
     * 增量任务（文件列表增删改）成功后调用此方法。
     * 不更新 version 是为了防止：在 recoveryPending 期间，增量任务提升了 version，
     * 导致后续恢复任务的版本判断出错。
     */
    void saveLastUpdateTime(const QDateTime &lastUpdateTime) const;

    /**
     * @brief 读取 Create 续建缓存的文件列表（线程安全）
     * @return 缓存的文件路径列表，若不存在或已清除则返回空列表
     *
     * 由 worker 线程在 CreateResumeHandler / resolveFileListForCreateResume 中调用，
     * 与 setCreateFileListCache（可能在主线程调用）通过内部互斥锁保证线程安全。
     */
    QStringList createFileListCache() const;

    /**
     * @brief 设置 Create 续建缓存的文件列表（线程安全）
     * @param cache 从 ANYTHING 获取的全量文件路径列表
     *
     * 调用场景：
     * - 主线程 startTask 启动新 Create 时清除旧缓存（传空列表）
     * - 主线程 finalizeIndexState 全量任务成功后清除缓存（传空列表）
     * - worker 线程 CreateIndexHandler / resolveFileListForCreateResume 中缓存文件列表
     */
    void setCreateFileListCache(const QStringList &cache) const;

    /**
     * @brief 读取 Create 续建的检查点偏移（线程安全）
     * @return 已处理的文件索引偏移，0 表示从头开始
     *
     * 使用 QAtomicInteger 保证跨线程可见性。
     */
    int createCheckpoint() const;

    /**
     * @brief 设置 Create 续建的检查点偏移（线程安全）
     * @param checkpoint 已处理的文件索引偏移
     *
     * 由 worker 线程在遍历文件时每处理一个文件更新一次。
     */
    void setCreateCheckpoint(int checkpoint) const;

private:
    IndexProfile m_profile;
    mutable QStringList m_createFileListCache;
    mutable QMutex m_createFileListCacheMutex;
    mutable QAtomicInteger<int> m_createCheckpoint { 0 };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXSTATESTORE_H
