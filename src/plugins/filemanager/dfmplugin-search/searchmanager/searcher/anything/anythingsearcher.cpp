// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anythingsearcher.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

static int kEmitInterval = 50;   // 推送时间间隔（ms）

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

static QString extract(const QString &pathWithMetadata)
{
    const qsizetype pos = pathWithMetadata.indexOf("<\\>");
    return (pos != -1) ? pathWithMetadata.left(pos) : pathWithMetadata;
}

static QStringList batchExtract(const QStringList &paths)
{
    QStringList result;
    result.reserve(paths.size());

    std::transform(paths.cbegin(), paths.cend(),
                   std::back_inserter(result),
                   [](const QString &path) { return extract(path); });

    return result;
}

AnythingSearcher::AnythingSearcher(const QUrl &url, const QString &keyword, bool isBindPath, QObject *parent)
    : AbstractSearcher(url, keyword, parent),
      isBindPath(isBindPath)
{
    anythingInterface = new QDBusInterface("com.deepin.anything",
                                           "/com/deepin/anything",
                                           "com.deepin.anything",
                                           QDBusConnection::systemBus(),
                                           this);
}

AnythingSearcher::~AnythingSearcher()
{
}

bool AnythingSearcher::search()
{
    // 准备状态切运行中，否则直接返回
    if (!status.testAndSetRelease(kReady, kRuning))
        return false;

    auto searchPath = UrlRoute::urlToPath(searchUrl);
    if (isBindPath) {
        originalPath = searchPath;
        searchPath = FileUtils::bindPathTransform(searchPath, true);
    }

    if (searchPath.isEmpty() || keyword.isEmpty()) {
        status.storeRelease(kCompleted);
        return false;
    }

    notifyTimer.start();

    // 直接调用新的search接口
    const QDBusPendingReply<QStringList> reply = anythingInterface->asyncCallWithArgumentList("search", { searchPath, keyword });
    auto results = reply.value();

    if (reply.error().type() != QDBusError::NoError) {
        fmWarning() << "deepin-anything search failed:"
                    << QDBusError::errorString(reply.error().type())
                    << reply.error().message();
        status.storeRelease(kCompleted);
        return false;
    }

    QHash<QString, QSet<QString>> hiddenFileHash;
    results = batchExtract(results);
    for (auto &item : results) {
        // 中断
        if (status.loadAcquire() != kRuning)
            return false;

        if (!SearchHelper::instance()->isHiddenFile(item, hiddenFileHash, searchPath)) {
            // 搜索路径还原
            if (isBindPath && item.startsWith(searchPath))
                item = item.replace(searchPath, originalPath);
            QMutexLocker lk(&mutex);
            allResults << QUrl::fromLocalFile(item);
        }

        // 推送
        tryNotify();
    }

    // 检查是否还有数据
    if (status.testAndSetRelease(kRuning, kCompleted)) {
        // 发送数据
        if (hasItem())
            emit unearthed(this);
    }

    return true;
}

void AnythingSearcher::stop()
{
    status.storeRelease(kTerminated);
}

bool AnythingSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

QList<QUrl> AnythingSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

void AnythingSearcher::tryNotify()
{
    qint64 cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > kEmitInterval) {
        lastEmit = cur;
        fmDebug() << "unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

bool AnythingSearcher::isSupported(const QUrl &url, bool &isBindPath)
{
    if (!url.isValid() || UrlRoute::isVirtual(url))
        return false;

    static QDBusInterface anything("com.deepin.anything",
                                   "/com/deepin/anything",
                                   "com.deepin.anything",
                                   QDBusConnection::systemBus());
    if (!anything.isValid())
        return false;

    return true;
}
