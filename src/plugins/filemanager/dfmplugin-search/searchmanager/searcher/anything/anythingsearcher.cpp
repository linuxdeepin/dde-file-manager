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
static qint32 kMaxCount = 100;   // 最大搜索结果数量
static qint64 kMaxTime = 500;   // 最大搜索时间（ms）

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

AnythingSearcher::AnythingSearcher(const QUrl &url, const QString &keyword, bool isBindPath, QObject *parent)
    : AbstractSearcher(url, SearchHelper::instance()->checkWildcardAndToRegularExpression(keyword), parent),
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
    //准备状态切运行中，否则直接返回
    if (!status.testAndSetRelease(kReady, kRuning))
        return false;

    QStringList searchDirList;
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
    // 如果挂载在此路径下的其它目录也支持索引数据, 则一并搜索
    QDBusPendingReply<QStringList> dirs = anythingInterface->asyncCallWithArgumentList("hasLFTSubdirectories", { searchPath });
    searchDirList << dirs.value();
    if (searchDirList.isEmpty() || searchDirList.first() != searchPath)
        searchDirList.prepend(searchPath);

    uint32_t startOffset = 0;
    uint32_t endOffset = 0;
    QHash<QString, QSet<QString>> hiddenFileHash;
    while (!searchDirList.isEmpty()) {
        //中断
        if (status.loadAcquire() != kRuning)
            return false;

        QList<QVariant> argumentList { kMaxCount, kMaxTime, startOffset, endOffset, searchDirList.first(), keyword, true };
        const QDBusPendingReply<QStringList, uint, uint> &reply = anythingInterface->asyncCallWithArgumentList("search", argumentList);
        auto results = reply.argumentAt<0>();
        if (reply.error().type() != QDBusError::NoError) {
            fmWarning() << "deepin-anything search failed:"
                       << QDBusError::errorString(reply.error().type())
                       << reply.error().message();
            startOffset = endOffset = 0;
            searchDirList.removeAt(0);
            continue;
        }

        startOffset = reply.argumentAt<1>();
        endOffset = reply.argumentAt<2>();

        for (auto &item : results) {
            // 中断
            if (status.loadAcquire() != kRuning)
                return false;

            if (!SearchHelper::instance()->isHiddenFile(item, hiddenFileHash, searchDirList.first())) {
                // 搜索路径还原
                if (isBindPath && item.startsWith(searchPath))
                    item = item.replace(searchPath, originalPath);
                QMutexLocker lk(&mutex);
                allResults << QUrl::fromLocalFile(item);
            }

            // 推送
            tryNotify();
        }

        // 当前目录已经搜索到了结尾
        if (startOffset >= endOffset) {
            startOffset = endOffset = 0;
            searchDirList.removeAt(0);
        }
    }

    //检查是否还有数据
    if (status.testAndSetRelease(kRuning, kCompleted)) {
        //发送数据
        if (hasItem())
            emit unearthed(this);
    }

    return true;
}

// void AnythingSearcher::stop()
// {
//     status.storeRelease(kTerminated);
// }

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

    auto path = UrlRoute::urlToPath(url);
    QDBusPendingReply<bool> reply = anything.asyncCallWithArgumentList("hasLFT", { path });
    if (reply.value())
        return true;

    const auto &bindPath = FileUtils::bindPathTransform(path, true);
    if (bindPath != path) {
        reply = anything.asyncCallWithArgumentList("hasLFT", { bindPath });
        if (reply.value()) {
            isBindPath = true;
            return true;
        }
    }

    return false;
}
