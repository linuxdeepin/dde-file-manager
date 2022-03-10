/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "fileextendattribthread.h"
#include "private/fileextendattribthreadprivate.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localdiriterator.h"

DFMBASE_USE_NAMESPACE

FileExtendAttribThreadPrivate::FileExtendAttribThreadPrivate(FileExtendAttribThread *extendAttribThread)
    : fileExtendAttibThread(extendAttribThread),
      state(FileExtendAttribThread::kStoppedState)
{
}

FileExtendAttribThreadPrivate::~FileExtendAttribThreadPrivate()
{
}

FileExtendAttribThread::FileExtendAttribThread(QObject *parent)
    : QThread(parent),
      dp(new FileExtendAttribThreadPrivate(this))
{
}

FileExtendAttribThread::~FileExtendAttribThread()
{
    stopThread();
}

void FileExtendAttribThread::startThread(const QList<QUrl> &urls)
{
    if (dp->state == FileExtendAttribThread::kRunningState)
        return;
    dp->totalSize = 0;
    dp->urlList = urls;
    dp->state = FileExtendAttribThread::kRunningState;
    start();
}

void FileExtendAttribThread::stopThread()
{
    if (dp->state == FileExtendAttribThread::kStoppedState) {
        return;
    }

    dp->state = FileExtendAttribThread::kStoppedState;
}

/*!
 * \brief           Calculate file size
 * \param url       QUrl of file
 */
void FileExtendAttribThread::dirSizeProcess(const QUrl &url)
{
    QString error;
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;

    qint64 size = 0;

    if (info->isFile()) {
        if (info->url() == QUrl::fromLocalFile("/proc/kcore") || info->url() == QUrl::fromLocalFile("/dev/core")) {
            return;
        }

        if (info->isSymLink() && (info->symLinkTarget() == QStringLiteral("/proc/kcore") || info->symLinkTarget() == QStringLiteral("/dev/core"))) {
            return;
        }

        AbstractFileInfo::FileType type = info->fileType();
        if (type == AbstractFileInfo::kCharDevice) {
            return;
        }

        if (type == AbstractFileInfo::kBlockDevice) {
            return;
        }

        if (type == AbstractFileInfo::kFIFOFile) {
            return;
        }

        if (type == AbstractFileInfo::kSocketFile) {
            return;
        }

        if (type == AbstractFileInfo::kUnknown) {
            return;
        }

        size = info->size();
        if (size > 0) {
            dp->totalSize += size;
            emit sigDirSizeChange(dp->totalSize);
        }
    } else if (info->isDir()) {
        dp->dirList << url;
    }
}

void FileExtendAttribThread::run()
{
    QString error {};
    for (const QUrl &url : dp->urlList) {
        const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<LocalDirIterator>(url, &error);
        if (!error.isEmpty()) {
            qWarning() << error;
            continue;
        }

        if (!iterator) {
            qWarning() << "Failed on create dir iterator, for url:" << url;
            continue;
        }

        bool status = dp->state == FileExtendAttribThread::kRunningState;
        while (iterator->hasNext() && status) {
            QUrl url = iterator->next();
            dirSizeProcess(url);
        }

        if (dp->state == FileExtendAttribThread::kStoppedState)
            return;
    }

    error.clear();
    while (!dp->dirList.isEmpty() && dp->state == FileExtendAttribThread::kRunningState) {
        const QUrl &directory_url = dp->dirList.dequeue();
        const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<LocalDirIterator>(directory_url, &error);
        if (!error.isEmpty()) {
            qWarning() << error;
            continue;
        }

        if (!iterator) {
            qWarning() << "Failed on create dir iterator, for url:" << directory_url;
            continue;
        }

        while (iterator->hasNext() && dp->state == FileExtendAttribThread::kRunningState) {
            QUrl url = iterator->next();
            dirSizeProcess(url);
        }
    }

    if (dp->totalSize == 0) {
        emit sigDirSizeChange(dp->totalSize);
        stopThread();
        return;
    }
    stopThread();
}
