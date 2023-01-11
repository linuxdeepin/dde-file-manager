/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fileinfohelper.h"

#include <QGuiApplication>
#include <QTimer>

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

void FileInfoHelper::fileRefreshAsync(const QUrl &url, const QSharedPointer<dfmio::DFileInfo> dfileInfo)
{
    if (stoped)
        return;

    emit fileInfoRefresh(url, dfileInfo);
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
}
