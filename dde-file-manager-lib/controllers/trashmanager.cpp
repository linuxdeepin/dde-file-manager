/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "trashmanager.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dfileproxywatcher.h"
#include "dfileinfo.h"
#include "models/trashfileinfo.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"

#include "interfaces/dfmstandardpaths.h"
#include "singleton.h"

#include "dfmeventdispatcher.h"

#include <QDebug>
#include <QUrlQuery>
#include <QTimer>
#include <QCoreApplication>
#include <QThread>

class TrashDirIterator : public DDirIterator
{
public:
    TrashDirIterator(const DUrl &url,
                     const QStringList &nameFilters,
                     QDir::Filters filter,
                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    DUrl fileUrl() const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    DUrl url() const Q_DECL_OVERRIDE;

private:
    QDirIterator *iterator;
};

TrashDirIterator::TrashDirIterator(const DUrl &url, const QStringList &nameFilters,
                                   QDir::Filters filter, QDirIterator::IteratorFlags flags)
    : DDirIterator()
{
    iterator = new QDirIterator(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + url.path(), nameFilters, filter, flags);
}

DUrl TrashDirIterator::next()
{
    return DUrl::fromTrashFile(DUrl::fromLocalFile(iterator->next()).path().remove(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath)));
}

bool TrashDirIterator::hasNext() const
{
    return iterator->hasNext();
}

QString TrashDirIterator::fileName() const
{
    return iterator->fileName();
}

DUrl TrashDirIterator::fileUrl() const
{
    return DUrl::fromTrashFile(iterator->filePath().remove(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath)));
}

const DAbstractFileInfoPointer TrashDirIterator::fileInfo() const
{
    return DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl());
}

DUrl TrashDirIterator::url() const
{
    return DUrl::fromTrashFile(iterator->path().remove(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath)));
}

TrashManager::TrashManager(QObject *parent)
    : DAbstractFileController(parent),
      m_trashFileWatcher(new DFileWatcher(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath),this))
{
    m_isTrashEmpty = isEmpty();
    QString trashFilePath = DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath);
    //make sure trash file exists
    if(!QFile::exists(trashFilePath))
        QDir().mkdir(trashFilePath);

    connect(m_trashFileWatcher, &DFileWatcher::fileDeleted, this, &TrashManager::trashFilesChanged);
    connect(m_trashFileWatcher, &DFileWatcher::subfileCreated, this, &TrashManager::trashFilesChanged);
    m_trashFileWatcher->startWatcher();
}

const DAbstractFileInfoPointer TrashManager::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new TrashFileInfo(event->url()));
}

bool TrashManager::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    Q_UNUSED(event)

    qWarning() << "trash open action is disable : " << event->url();
//    return FileServices::instance()->openFile(DUrl::fromLocalFile(TRASHFILEPATH + fileUrl.path()));
    return false;
}

DUrlList TrashManager::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    fileService->deleteFiles(event->sender(), event->urlList(), true, false, true);

    return DUrlList();
}

bool TrashManager::restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const
{
    DUrlList originUrls;

    bool ok = restoreTrashFile(event->urlList(), &originUrls);

    if (ok && !originUrls.isEmpty()) {
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, originUrls));
    }

    return ok;
}

bool TrashManager::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    if (event->action() != DFMGlobal::CopyAction) {
        event->ignore();
        return false;
    }

    DUrlList localList;

    for(const DUrl &url : event->urlList()) {
        const QString &path = url.path();

        localList << DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + path);

        // blumia: shouldn't do this since we are just copy file out. It's okay if the file name is a hashed name.
        //         use the restore function provided by right click menu to perform restore behavior, not copy-paste.
//        if(path.lastIndexOf('/') > 0) {
//            localList << DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath) + path);
//        }
    }

    fileService->writeFilesToClipboard(event->sender(), event->action(), localList);

    return true;
}

DUrlList TrashManager::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    if (event->action() != DFMGlobal::CutAction || event->targetUrl() != DUrl::fromTrashFile("/")) {
        return DUrlList();
    }

    if (event->urlList().isEmpty())
        return DUrlList();

    return fileService->moveToTrash(event->sender(), event->urlList());
}


bool TrashManager::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList localList;

    for(const DUrl &url : event->urlList()) {
        if (DUrl::fromTrashFile("/") == url) {
            cleanTrash(event->sender());
            return true;
        }

        const QString &path = url.path();

        localList << DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + path);

        if (path.lastIndexOf('/') == 0) {
            localList << DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath) + path + ".trashinfo");
        }
    }

    fileService->deleteFiles(event->sender(), localList, false, event->silent());

    return true;
}

const DDirIteratorPointer TrashManager::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new TrashDirIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
}

namespace TrashManagerPrivate {
DUrl localToTrash(const DUrl &url)
{
    const QString &localPath = url.toLocalFile();
    const QString &trashPath = DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath);

    if (!localPath.startsWith(trashPath))
        return DUrl();

    DUrl u = DUrl::fromTrashFile(localPath.mid(trashPath.length()));

    if (u.path().isEmpty())
        u.setPath("/");

    return u;
}
QString trashToLocal(const DUrl &url)
{
    return DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + url.path();
}
}

DAbstractFileWatcher *TrashManager::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFileProxyWatcher(event->url(),
                                 new DFileWatcher(TrashManagerPrivate::trashToLocal(event->url())),
                                 TrashManagerPrivate::localToTrash);
}

bool TrashManager::restoreTrashFile(const DUrlList &list, DUrlList *restoreOriginUrls)
{
    bool ok = true;

    DUrlList restoreFailedList;
    DUrlList restoreFileOriginUrlList;

    for (const DUrl &url : list) {
        if (url == DUrl::fromTrashFile("/")) {
            // restore all
            DUrlList list;

            for (const DAbstractFileInfoPointer &info : DFileService::instance()->getChildren(Q_NULLPTR, DUrl::fromTrashFile("/"), QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System))
                list << info->fileUrl();

            if (list.isEmpty())
                return true;

            return restoreTrashFile(list, restoreOriginUrls);
        }

        //###(zccrs): 必须通过 DAbstractFileInfoPointer 使用
        //            因为对象会被缓存，所以有可能在其它线程中被使用
        //            如果直接定义一个TrashFileInfo对象，就可能存在对象被重复释放
        QExplicitlySharedDataPointer<TrashFileInfo> info(new TrashFileInfo(url));

        bool ret = info->restore();

        if (!ret && info->exists()) {
            restoreFailedList << info->fileUrl();
        } else {
            restoreFileOriginUrlList << info->originUrl();
        }

        ok = ok && ret;
    }

    if (!ok && restoreFailedList.count() > 0){
        emit fileSignalManager->requestShowRestoreFailedDialog(restoreFailedList);
    }

    if (restoreOriginUrls)
        *restoreOriginUrls = restoreFileOriginUrlList;

    return ok;
}

void TrashManager::cleanTrash(const QObject *sender) const
{
    DUrlList list;
    const DUrl &file_url = DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath));
    const DUrl &info_url = DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath));

    if (QFile::exists(file_url.toLocalFile())) {
        list << file_url;
    }

    if (QFile::exists(info_url.toLocalFile())) {
        list << info_url;
    }

    fileService->deleteFiles(sender, list, false, false, true);
}

bool TrashManager::isEmpty()
{
    QDir dir(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath));

    if (!dir.exists())
        return true;

    dir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

    QDirIterator iterator(dir);

    return !iterator.hasNext();
}

void TrashManager::trashFilesChanged(const DUrl& url)
{
    Q_UNUSED(url);
    if(m_isTrashEmpty == isEmpty())
        return;

    m_isTrashEmpty = isEmpty();
    emit fileSignalManager->trashStateChanged();
}
