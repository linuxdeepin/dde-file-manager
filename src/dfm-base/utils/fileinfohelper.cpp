// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileinfohelper.h"

#include <QGuiApplication>
#include <QTimer>

Q_DECLARE_METATYPE(QSharedPointer<dfmio::DFileInfo>);

DFMBASE_USE_NAMESPACE
FileInfoHelper::FileInfoHelper(QObject *parent)
    : QObject(parent), thread(new QThread), worker(new FileInfoAsycWorker)
{
    init();
}

void FileInfoHelper::init()
{
    // connect quit app to stop
    connect(qApp, &QGuiApplication::aboutToQuit, this, &FileInfoHelper::aboutToQuit);
    // connect thumb

    // connect file info async worker
    connect(this, &FileInfoHelper::fileCount, worker.data(), &FileInfoAsycWorker::fileConutAsync, Qt::QueuedConnection);
    connect(worker.data(), &FileInfoAsycWorker::fileConutAsyncFinish, this, &FileInfoHelper::fileCountFinished, Qt::QueuedConnection);
    connect(this, &FileInfoHelper::fileMimeType, worker.data(), &FileInfoAsycWorker::fileMimeType, Qt::QueuedConnection);
    connect(this, &FileInfoHelper::fileInfoRefresh, worker.data(), &FileInfoAsycWorker::fileRefresh, Qt::QueuedConnection);
    connect(worker.data(), &FileInfoAsycWorker::fileMimeTypeFinished, this, &FileInfoHelper::fileMimeTypeFinished, Qt::QueuedConnection);
    connect(this, &FileInfoHelper::fileThumb, worker.data(), &FileInfoAsycWorker::fileThumb, Qt::QueuedConnection);
    connect(worker.data(), &FileInfoAsycWorker::createThumbnailFinished,
            this, &FileInfoHelper::createThumbnailFinished, Qt::QueuedConnection);
    connect(worker.data(), &FileInfoAsycWorker::createThumbnailFailed,
            this, &FileInfoHelper::createThumbnailFailed, Qt::QueuedConnection);

    worker->moveToThread(thread.data());
    thread->start();
}

void FileInfoHelper::threadHandleDfmFileInfo(const QSharedPointer<dfmio::DFileInfo> dfileInfo)
{
    auto future = dfileInfo->refreshAsync();
    future.waitForFinished();
    emit fileRefreshFinished(dfileInfo->uri(), false);
    if (dfileInfo->attribute(dfmio::DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        QUrl tar =
                QUrl::fromLocalFile(dfileInfo->attribute(dfmio::DFileInfo::AttributeID::kStandardSymlinkTarget).toString());
        QWriteLocker lk(&symLinkHashLock);
        symLinkHash.insert(tar, dfileInfo->uri());
    } else {
        QWriteLocker lk(&symLinkHashLock);
        QUrl org = symLinkHash.take(dfileInfo->uri());
        if (org.isValid())
            emit fileRefreshFinished(org, true);
    }
}

QSharedPointer<FileInfoHelperUeserData> FileInfoHelper::fileCountAsync(QUrl &url)
{
    if (stoped)
        return nullptr;
    QSharedPointer<FileInfoHelperUeserData> data(new FileInfoHelperUeserData);
    emit fileCount(url, data);
    return data;
}

QSharedPointer<FileInfoHelperUeserData> FileInfoHelper::fileMimeTypeAsync(const QUrl &url, const QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs)
{
    if (stoped)
        return nullptr;
    QSharedPointer<FileInfoHelperUeserData> data(new FileInfoHelperUeserData);
    emit fileMimeType(url, mode, inod, isGvfs, data);
    return data;
}

QSharedPointer<FileInfoHelperUeserData> FileInfoHelper::fileThumbAsync(const QUrl &url, ThumbnailProvider::Size size)
{
    if (stoped)
        return nullptr;
    static constexpr uint16_t kRequestThumbnailDealy { 500 };
    QSharedPointer<FileInfoHelperUeserData> data(new FileInfoHelperUeserData);
    QUrl thumbUrl(url);
    QTimer::singleShot(kRequestThumbnailDealy, [thumbUrl, size, data]() {
        if (FileInfoHelper::instance().stoped)
            return;
        emit FileInfoHelper::instance().fileThumb(thumbUrl, size, data);
    });

    return data;
}

void FileInfoHelper::fileRefreshAsync(const QSharedPointer<dfmio::DFileInfo> dfileInfo)
{
    if (stoped)
        return;
    QtConcurrent::run(&pool, [this, dfileInfo]() {
        threadHandleDfmFileInfo(dfileInfo);
    });
}

FileInfoHelper::~FileInfoHelper()
{
    aboutToQuit();
}

FileInfoHelper &FileInfoHelper::instance()
{
    static FileInfoHelper helper;
    return helper;
}

void FileInfoHelper::aboutToQuit()
{
    stoped = true;
    thread->quit();
    worker->stopWorker();
    thread->wait();
    pool.waitForDone();
}
