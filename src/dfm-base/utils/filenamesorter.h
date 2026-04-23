// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMESORTER_H
#define FILENAMESORTER_H

#include <dfm-base/dfm_base_global.h>

#include <QCollator>
#include <QUrl>
#include <QStringList>
#include <algorithm>

DFMBASE_BEGIN_NAMESPACE

/**
 * @class FileNameSorter
 * @brief 高性能文件名排序工具类
 *
 * 使用 QCollator::sortKey() 预处理文件名，提供批量排序的高性能实现。
 * 线程安全：每个线程拥有独立的 QCollator 实例。
 *
 * 设计原则：
 * - 单一职责：只负责文件名排序
 * - 高内聚低耦合：不依赖外部模块
 * - KISS：接口简洁明了
 */
class FileNameSorter
{
public:
    /**
     * @brief 获取当前线程的 QCollator 实例
     * @return 线程局部 QCollator 引用
     *
     * 配置：numericMode=true, caseSensitivity=CaseSensitive
     */
    static QCollator &collator();

    /**
     * @brief 生成文件名的排序键
     * @param fileName 文件名
     * @return QCollatorSortKey 可用于高效比较
     */
    static QCollatorSortKey sortKey(const QString &fileName);

    /**
     * @brief 批量排序字符串列表（使用 sortKey 优化）
     * @param fileNames 文件名列表（原地排序）
     * @param order 排序顺序（默认升序）
     */
    static void sort(QStringList &fileNames, Qt::SortOrder order = Qt::AscendingOrder);

    /**
     * @brief 批量排序 URL 列表（按文件名）
     * @param urls URL 列表（原地排序）
     * @param order 排序顺序（默认升序）
     */
    static void sortUrls(QList<QUrl> &urls, Qt::SortOrder order = Qt::AscendingOrder);

    /**
     * @brief 比较两个文件名（用于非批量场景）
     * @param left 左侧文件名
     * @param right 右侧文件名
     * @param order 排序顺序
     * @return true 如果 left 应该排在 right 前面
     */
    static bool compare(const QString &left, const QString &right,
                        Qt::SortOrder order = Qt::AscendingOrder);

    /**
     * @brief 使用预生成的 sortKey 进行排序
     * @tparam T 元素类型
     * @tparam GetKey 获取 sortKey 的函数
     * @param items 元素列表
     * @param getKey 从元素获取 sortKey 的函数
     * @param order 排序顺序
     *
     * 使用示例：
     * @code
     * QList<QPair<QString, QCollatorSortKey>> items;
     * for (const auto &url : urls) {
     *     items.append({url.fileName(), FileNameSorter::sortKey(url.fileName())});
     * }
     * FileNameSorter::sortByKey(items, [](const auto &item) { return item.second; });
     * @endcode
     */
    template<typename Container, typename GetKey>
    static void sortByKey(Container &items, GetKey getKey, Qt::SortOrder order = Qt::AscendingOrder)
    {
        if (order == Qt::AscendingOrder) {
            std::stable_sort(items.begin(), items.end(), [&getKey](const auto &a, const auto &b) {
                return getKey(a) < getKey(b);
            });
        } else {
            std::stable_sort(items.begin(), items.end(), [&getKey](const auto &a, const auto &b) {
                return getKey(b) < getKey(a);
            });
        }
    }

private:
    FileNameSorter() = delete;
    ~FileNameSorter() = delete;
    FileNameSorter(const FileNameSorter &) = delete;
    FileNameSorter &operator=(const FileNameSorter &) = delete;
};

DFMBASE_END_NAMESPACE

#endif   // FILENAMESORTER_H
