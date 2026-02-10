// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailfactory.h"
#include "thumbnailcreators.h"
#include "thumbnailhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <QGuiApplication>

using namespace dfmbase;
DFMGLOBAL_USE_NAMESPACE

static constexpr int kMaxCountLimit { 50 };
static constexpr int kPushInterval { 100 };   // ms

ThumbnailFactory::ThumbnailFactory(QObject *parent)
    : QObject(parent),
      thread(new QThread),
      worker(new ThumbnailWorker)
{
    qCInfo(logDFMBase) << "thumbnail: ThumbnailFactory initializing with" << QThread::idealThreadCount() << "ideal thread count";

    registerThumbnailCreator(Mime::kTypeImageVDjvu, ThumbnailCreators::djvuThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeImageVDMultipage, ThumbnailCreators::djvuThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeTextPlain, ThumbnailCreators::textThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeAppPdf, ThumbnailCreators::pdfThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeAppVRRMedia, ThumbnailCreators::videoThumbnailCreatorFfmpeg);
    registerThumbnailCreator("image/*", ThumbnailCreators::imageThumbnailCreator);
    registerThumbnailCreator("audio/*", ThumbnailCreators::audioThumbnailCreator);
    registerThumbnailCreator("video/*", ThumbnailCreators::videoThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeAppAppimage, ThumbnailCreators::appimageThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeAppUab, ThumbnailCreators::uabThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeAppPptx, ThumbnailCreators::pptxThumbnailCreator);

    init();
}

ThumbnailFactory::~ThumbnailFactory()
{
    qCInfo(logDFMBase) << "thumbnail: ThumbnailFactory destructor called";
    if (thread->isRunning())
        onAboutToQuit();
}

void ThumbnailFactory::init()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    taskPushTimer.setSingleShot(true);
    taskPushTimer.setInterval(kPushInterval);
    connect(&taskPushTimer, &QTimer::timeout, this, &ThumbnailFactory::pushTask);
    connect(this, &ThumbnailFactory::thumbnailJob, this, &ThumbnailFactory::doJoinThumbnailJob, Qt::QueuedConnection);
    connect(qApp, &QGuiApplication::aboutToQuit, this, &ThumbnailFactory::onAboutToQuit);

    connect(this, &ThumbnailFactory::addTask, worker.data(), &ThumbnailWorker::onTaskAdded, Qt::QueuedConnection);
    connect(worker.data(), &ThumbnailWorker::thumbnailCreateFinished, this, &ThumbnailFactory::produceFinished, Qt::QueuedConnection);
    connect(worker.data(), &ThumbnailWorker::thumbnailCreateFailed, this, &ThumbnailFactory::produceFailed, Qt::QueuedConnection);

    worker->moveToThread(thread.data());
    thread->start();

    qCInfo(logDFMBase) << "thumbnail: ThumbnailFactory initialized, worker thread started";
}

void ThumbnailFactory::joinThumbnailJob(const QUrl &url, ThumbnailSize size)
{
    if (QThread::currentThread() != qApp->thread()) {
        qCDebug(logDFMBase) << "thumbnail: cross-thread job request, queuing for:" << url;
        emit thumbnailJob(url, size);
        return;
    }
    doJoinThumbnailJob(url, size);
}

bool ThumbnailFactory::registerThumbnailCreator(const QString &mimeType, ThumbnailCreator creator)
{
    Q_ASSERT(creator);
    bool success = worker->registerCreator(mimeType, creator);
    if (success) {
        qCDebug(logDFMBase) << "thumbnail: registered creator for mime type:" << mimeType;
    } else {
        qCWarning(logDFMBase) << "thumbnail: failed to register creator for mime type:" << mimeType;
    }
    return success;
}

void ThumbnailFactory::onAboutToQuit()
{
    qCInfo(logDFMBase) << "thumbnail: application about to quit, stopping worker and thread";
    worker->stop();
    thread->quit();
    bool finished = thread->wait(3000);
    if (!finished) {
        qCWarning(logDFMBase) << "thumbnail: worker thread did not finish within 3 seconds, forcing termination";
        thread->terminate();
        thread->wait(1000);
    } else {
        qCInfo(logDFMBase) << "thumbnail: worker thread stopped gracefully";
    }
}

void ThumbnailFactory::pushTask()
{
    auto map = std::move(taskMap);
    qCDebug(logDFMBase) << "thumbnail: pushing" << map.size() << "tasks to worker thread";
    emit addTask(map);
}

void ThumbnailFactory::doJoinThumbnailJob(const QUrl &url, ThumbnailSize size)
{
    if (FileUtils::containsCopyingFileUrl(url)) {
        qCDebug(logDFMBase) << "thumbnail: skipping file being copied:" << url;
        return;
    }

    if (taskMap.contains(url)) {
        return;
    }

    if (taskMap.isEmpty()) {
        taskPushTimer.start();
    }

    taskMap.insert(url, size);

    if (taskMap.size() < kMaxCountLimit)
        return;

    qCDebug(logDFMBase) << "thumbnail: task queue reached limit" << kMaxCountLimit << ", pushing immediately";
    pushTask();
}
