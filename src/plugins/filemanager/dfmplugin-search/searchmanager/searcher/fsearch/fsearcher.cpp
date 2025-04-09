// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsearcher.h"
#include "fsearchhandler.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/urlroute.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

static constexpr int kEmitInterval = 50;   // 推送时间间隔（ms）

FSearcher::FSearcher(const QUrl &url, const QString &key, QObject *parent)
    : AbstractSearcher(url, SearchHelper::instance()->checkWildcardAndToRegularExpression(key), parent),
      searchHandler(new FSearchHandler)
{
    searchHandler->init();
    searchHandler->setFlags(FSearchHandler::FSEARCH_FLAG_REGEX | FSearchHandler::FSEARCH_FLAG_FILTER_HIDDEN_FILE);
}

FSearcher::~FSearcher()
{
    if (searchHandler) {
        delete searchHandler;
        searchHandler = nullptr;
    }
}

bool FSearcher::isSupport(const QUrl &url)
{
    if (!url.isValid() || UrlRoute::isVirtual(url))
        return false;

    auto path = UrlRoute::urlToPath(url);
    return FSearchHandler::checkPathSearchable(path);
}

bool FSearcher::search()
{
    //准备状态切运行中，否则直接返回
    if (!status.testAndSetRelease(kReady, kRuning))
        return false;

    const QString &path = UrlRoute::urlToPath(searchUrl);
    if (path.isEmpty() || keyword.isEmpty()) {
        status.storeRelease(kCompleted);
        return false;
    }

    notifyTimer.start();
    searchHandler->loadDatabase(path, "");
    auto callback = std::bind(FSearcher::receiveResultCallback, std::placeholders::_1, std::placeholders::_2, this);

    conditionMtx.lock();
    if (searchHandler->search(keyword, callback))
        waitCondition.wait(&conditionMtx, ULONG_MAX);
    conditionMtx.unlock();

    if (status.testAndSetRelease(kRuning, kCompleted)) {
        if (hasItem())
            emit unearthed(this);
    }

    return true;
}

// void FSearcher::stop()
// {
//     searchHandler->stop();
//     status.storeRelease(kTerminated);
// }

bool FSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

QList<QUrl> FSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

void FSearcher::tryNotify()
{
    qint64 cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > kEmitInterval) {
        lastEmit = cur;
        fmDebug() << "unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

void FSearcher::receiveResultCallback(const QString &result, bool isFinished, FSearcher *self)
{
    if (self->status.loadAcquire() != kRuning || isFinished) {
        self->conditionMtx.lock();
        self->waitCondition.wakeAll();
        self->conditionMtx.unlock();
        return;
    }

    if (!SearchHelper::instance()->isHiddenFile(result, self->hiddenFileHash, UrlRoute::urlToPath(self->searchUrl))) {
        QMutexLocker lk(&self->mutex);
        self->allResults << QUrl::fromLocalFile(result);
    }

    self->tryNotify();
}
