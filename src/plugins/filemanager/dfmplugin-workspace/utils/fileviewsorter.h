// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWSORTER_H
#define FILEVIEWSORTER_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/sortfileinfo.h>

#include <QCollator>
#include <QUrl>
#include <QVariant>
#include <QHash>
#include <functional>

DPWORKSPACE_BEGIN_NAMESPACE

// 前向声明
class FileItemData;
typedef QSharedPointer<FileItemData> FileItemDataPointer;

/**
 * @class FileViewSorter
 * @brief 文件视图高性能排序器
 *
 * 使用 QCollator::sortKey() 预处理排序数据，提供批量排序和增量插入定位功能。
 * 设计原则：
 * - 单一职责：专注于文件视图排序逻辑
 * - 高性能：使用 sortKey 避免重复比较
 * - 低耦合：独立于 FileSortWorker，可单独测试
 */
class FileViewSorter
{
public:
    /**
     * @brief 排序角色
     */
    enum class SortRole : uint8_t {
        FileName,   // 文件名（默认）
        Size,   // 大小
        LastModified,   // 最后修改时间
        LastCreated,   // 最后创建时间
        LastRead,   // 最后访问时间
        MimeType,   // 文件类型
        FilePath,   // 文件路径
        OriginalPath,   // 原始路径（垃圾桶）
        DeletionDate,   // 删除时间（垃圾桶）
    };

    /**
     * @brief 排序上下文信息
     */
    struct SortContext
    {
        QUrl rootUrl;   // 根目录 URL
        bool isMixDirAndFile = false;   // 是否混排目录和文件
        Qt::SortOrder order = Qt::AscendingOrder;   // 排序顺序
        SortRole role = SortRole::FileName;   // 排序角色

        // 显示名特殊处理标志（由 FileSortWorker 在外部判断）
        bool isUnderHomeDir = false;   // 当前目录是否在主目录下（XDG 目录转译）
        bool checkDesktopFile = false;   // 是否需要处理 desktop 文件显示名

        // 数据访问回调（由 FileSortWorker 提供）
        std::function<FileItemDataPointer(const QUrl &)> getDataCallback;
    };

public:
    FileViewSorter() = default;
    ~FileViewSorter() = default;

    /**
     * @brief 设置排序上下文
     */
    void setContext(const SortContext &context);

    /**
     * @brief 批量排序 URL 列表（使用 sortKey + 快排）
     * @param urls 待排序的 URL 列表
     * @return 排序后的 URL 列表
     */
    QList<QUrl> sort(const QList<QUrl> &urls);

    /**
     * @brief 简单反序列表
     * @param urls URL 列表
     * @return 反序后的列表
     */
    QList<QUrl> reverse(const QList<QUrl> &urls);

    /**
     * @brief 增量插入定位（使用 sortKey + 二分查找）
     * @param url 待插入的 URL
     * @param sortedList 已排序的列表
     * @return 插入位置索引
     */
    int findInsertPosition(const QUrl &url, const QList<QUrl> &sortedList);

    /**
     * @brief 从 ItemRoles 转换为 SortRole
     */
    static SortRole toItemRole(dfmbase::Global::ItemRoles role);

private:
    /**
     * @brief 生成排序键字符串
     */
    QString generateSortKeyString(const QUrl &url);

    /**
     * @brief 生成排序键字符串（不含目录/文件前缀）
     */
    QString generateSortKeyStringInternal(const QUrl &url);

    /**
     * @brief 获取排序数据
     */
    QVariant getSortData(const QUrl &url);

    /**
     * @brief 获取 MimeType 分组权重
     */
    int getMimeTypeGroupRank(const QString &mimeType);

    /**
     * @brief 编码 MimeType 排序键
     */
    QString encodeMimeTypeSortKey(const QString &mimeType, const QString &fileName);

    /**
     * @brief 检查 URL 是否为目录
     */
    bool isDir(const QUrl &url);

    /**
     * @brief 获取文件显示名（处理主目录特殊情况）
     */
    QString getFileDisplayName(const QUrl &url, const FileItemDataPointer &itemData);

    /**
     * @brief 分离排序（非混排状态：目录和文件分开排序）
     */
    QList<QUrl> sortSeparated(const QList<QUrl> &urls);

    /**
     * @brief 混合排序（混排状态或单组内部排序）
     */
    QList<QUrl> sortMixed(const QList<QUrl> &urls);

    /**
     * @brief 批量获取 MimeType（使用扩展名缓存优化）
     * @param urls URL 列表
     * @return URL -> MimeType 映射
     */
    QHash<QUrl, QString> batchGetMimeTypes(const QList<QUrl> &urls);

private:
    SortContext m_context;

    // 线程安全的 QCollator（每个线程独立）
    QCollator &collator();
};

DPWORKSPACE_END_NAMESPACE

#endif   // FILEVIEWSORTER_H
