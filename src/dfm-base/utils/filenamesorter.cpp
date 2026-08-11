// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenamesorter.h"
#include "collation/collationstrategy.h"
#include "collation/collationstrategyprovider.h"

#include <algorithm>

DFMBASE_BEGIN_NAMESPACE

QByteArray FileNameSorter::sortKey(const QString &fileName)
{
    return CollationStrategyProvider::instance()->strategy().sortKey(fileName);
}

void FileNameSorter::sort(QStringList &fileNames, Qt::SortOrder order)
{
    if (fileNames.size() <= 1)
        return;

    // 批次入口一次性获取策略，保证整个批次内策略一致（避免 dconfig 切换竞态）
    const CollationStrategy &s = CollationStrategyProvider::instance()->strategy();

    // 预生成所有 sortKey
    QVector<QPair<QString, QByteArray>> fileWithKeys;
    fileWithKeys.reserve(fileNames.size());
    for (const QString &name : fileNames) {
        fileWithKeys.emplace_back(qMakePair(name, s.sortKey(name)));
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

    // 批次入口一次性获取策略，保证整个批次内策略一致（避免 dconfig 切换竞态）
    const CollationStrategy &s = CollationStrategyProvider::instance()->strategy();

    // 预生成所有 sortKey
    QVector<QPair<QUrl, QByteArray>> urlWithKeys;
    urlWithKeys.reserve(urls.size());
    for (const QUrl &url : urls) {
        urlWithKeys.emplace_back(qMakePair(url, s.sortKey(url.fileName())));
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
    int result = CollationStrategyProvider::instance()->strategy().compare(left, right);
    return (order == Qt::AscendingOrder) ? (result < 0) : (result > 0);
}

DFMBASE_END_NAMESPACE
