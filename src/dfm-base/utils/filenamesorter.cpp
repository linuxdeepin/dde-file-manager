// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenamesorter.h"

#include <memory>
#include <algorithm>

DFMBASE_BEGIN_NAMESPACE

QCollator &FileNameSorter::collator()
{
    // 使用 thread_local + unique_ptr 确保线程安全和自动内存清理
    // 线程退出时，unique_ptr 自动析构，释放 QCollator 内存
    static thread_local std::unique_ptr<QCollator> collator;

    if (!collator) {
        collator = std::make_unique<QCollator>();
        collator->setNumericMode(true);   // 自然排序：file2 < file10
        collator->setCaseSensitivity(Qt::CaseSensitive);   // 大小写敏感
    }

    return *collator;
}

QCollatorSortKey FileNameSorter::sortKey(const QString &fileName)
{
    return collator().sortKey(fileName);
}

void FileNameSorter::sort(QStringList &fileNames, Qt::SortOrder order)
{
    if (fileNames.size() <= 1)
        return;

    // 预生成所有 sortKey
    QVector<QPair<QString, QCollatorSortKey>> fileWithKeys;
    fileWithKeys.reserve(fileNames.size());
    for (const QString &name : fileNames) {
        fileWithKeys.emplace_back(qMakePair(name, sortKey(name)));
    }

    // 使用 sortKey 排序
    sortByKey(
            fileWithKeys, [](const auto &item) { return item.second; }, order);

    // 提取排序后的文件名
    fileNames.clear();
    fileNames.reserve(fileWithKeys.size());
    for (const auto &item : fileWithKeys) {
        fileNames.append(item.first);
    }
}

void FileNameSorter::sortUrls(QList<QUrl> &urls, Qt::SortOrder order)
{
    if (urls.size() <= 1)
        return;

    // 预生成所有 sortKey
    QVector<QPair<QUrl, QCollatorSortKey>> urlWithKeys;
    urlWithKeys.reserve(urls.size());
    for (const QUrl &url : urls) {
        urlWithKeys.emplace_back(qMakePair(url, sortKey(url.fileName())));
    }

    // 使用 sortKey 排序
    sortByKey(
            urlWithKeys, [](const auto &item) { return item.second; }, order);

    // 提取排序后的 URL
    urls.clear();
    urls.reserve(urlWithKeys.size());
    for (const auto &item : urlWithKeys) {
        urls.append(item.first);
    }
}

bool FileNameSorter::compare(const QString &left, const QString &right, Qt::SortOrder order)
{
    int result = collator().compare(left, right);
    return (order == Qt::AscendingOrder) ? (result < 0) : (result > 0);
}

DFMBASE_END_NAMESPACE
