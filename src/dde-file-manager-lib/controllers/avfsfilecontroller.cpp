/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "avfsfilecontroller.h"
#include "interfaces/dfileinfo.h"
#include "dfmstandardpaths.h"
#include <QStandardPaths>
#include <QDir>
#include "models/avfsfileinfo.h"
#include "dfileinfo.h"
#include <QDirIterator>
#include <dfilewatcher.h>
#include "dfileservices.h"
#include "shutil/fileutils.h"
#include "dfmevent.h"

class AVFSIterator : public DDirIterator
{
public:
    AVFSIterator(const DUrl &url,
                 const QStringList &nameFilters,
                 QDir::Filters filter,
                 QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    ~AVFSIterator() override;

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;

private:
    QDirIterator *iterator;
    DUrl currentUrl;
};

AVFSIterator::AVFSIterator(const DUrl &url, const QStringList &nameFilters, QDir::Filters filter, QDirIterator::IteratorFlags flags):
    DDirIterator()
{
    QString realPath = AVFSFileInfo::realDirUrl(url).toLocalFile();
    iterator = new QDirIterator(realPath, nameFilters, filter, flags);
    currentUrl = url;
}

AVFSIterator::~AVFSIterator()
{
    delete iterator;
}

DUrl AVFSIterator::next()
{
    QString realPath = iterator->next();
    Q_UNUSED(realPath);
    DUrl url = DUrl::fromAVFSFile(currentUrl.path() + "/" + fileName());
    return url;
}

bool AVFSIterator::hasNext() const
{
    return iterator->hasNext();
}

QString AVFSIterator::fileName() const
{
    return fileInfo()->fileName();
}

DUrl AVFSIterator::fileUrl() const
{
    return fileInfo()->fileUrl();
}

const DAbstractFileInfoPointer AVFSIterator::fileInfo() const
{
    DUrl url = DUrl::fromAVFSFile(currentUrl.path() + "/" + iterator->fileName());
    return DAbstractFileInfoPointer(new AVFSFileInfo(url));
}

DUrl AVFSIterator::url() const
{
    return currentUrl;
}

AVFSFileController::AVFSFileController(QObject *parent):
    DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer AVFSFileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    DAbstractFileInfoPointer info(new AVFSFileInfo(event->url()));

    return info;
}

const DDirIteratorPointer AVFSFileController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new AVFSIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
}

DAbstractFileWatcher *AVFSFileController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    QString realPath = AVFSFileInfo::realDirUrl(event->url()).toLocalFile();

    return new DFileWatcher(realPath);
}

bool AVFSFileController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    return DFileService::instance()->openFileLocation(event->sender(), realUrl(event->url()));
}

bool AVFSFileController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), realUrl(event->url()));
}

bool AVFSFileController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), realUrl(event->url()));
}

bool AVFSFileController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    DUrlList realUrlList;

    foreach (const DUrl &url, event->urlList()) {
        realUrlList << realUrl(url);
    }

    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(), realUrlList);
}

bool AVFSFileController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    return DFileService::instance()->openInTerminal(event->sender(), realUrl(event->url()));
}

DUrl AVFSFileController::realUrl(const DUrl &url)
{
    return AVFSFileInfo::realFileUrl(url);
}

QString AVFSFileController::findArchFileRootPath(const DUrl &url)
{
    QStringList items = url.path().split("/");
    for (int i = 0; i < items.size(); i++) {
        QString path = "";
        for (int j = 0; j <= i; j++) {
            path += (items.at(j) + "/");
        }
        if (path.endsWith("/"))
            path.chop(1);
        if (FileUtils::isArchive(path))
            return path;
    }
    return QString();
}

