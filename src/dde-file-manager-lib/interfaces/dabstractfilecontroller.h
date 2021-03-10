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

#ifndef ABSTRACTFILECONTROLLER_H
#define ABSTRACTFILECONTROLLER_H

#include "dabstractfileinfo.h"
#include "ddiriterator.h"

#include <QObject>
#include <QDir>
#include <QDirIterator>

class DFMEvent;
class DFMOpenFileEvent;
class DFMOpenFilesEvent;
class DFMOpenFileByAppEvent;
class DFMOpenFilesByAppEvent;
class DFMCompressEvent;
class DFMDecompressEvent;
class DFMWriteUrlsToClipboardEvent;
class DFMRenameEvent;
class DFMDeleteEvent;
class DFMMoveToTrashEvent;
class DFMRestoreFromTrashEvent;
class DFMPasteEvent;
class DFMMkdirEvent;
class DFMTouchFileEvent;
class DFMSetPermissionEvent;
class DFMOpenFileLocation;
class DFMAddToBookmarkEvent;
class DFMRemoveBookmarkEvent;
class DFMCreateSymlinkEvent;
class DFMGetChildrensEvent;
class DFMCreateDiriterator;
class DFMCreateFileInfoEvent;
class DFMCreateFileWatcherEvent;
class DFMOpenInTerminalEvent;
class DFMFileShareEvent;
class DFMCancelFileShareEvent;
class DUrl;
class DAbstractFileWatcher;
class DFMSetFileTagsEvent;
class DFMRemoveTagsOfFileEvent;
class DFMGetTagsThroughFilesEvent;
class DFMSetFileExtraProperties;
class DFMUrlBaseEvent;

DFM_BEGIN_NAMESPACE
class DFileHandler;
class DFileDevice;
class DStorageInfo;
DFM_END_NAMESPACE

typedef QList<DUrl> DUrlList;
class DAbstractFileController : public QObject
{
    Q_OBJECT

public:
    explicit DAbstractFileController(QObject *parent = 0);

    virtual bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const;
    virtual bool openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const;
    virtual bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const;
    virtual bool openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const;
    virtual bool compressFiles(const QSharedPointer<DFMCompressEvent> &event) const;
    virtual bool decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const;
    virtual bool decompressFileHere(const QSharedPointer<DFMDecompressEvent> &event) const;
    virtual bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const;
    virtual bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const;
    virtual bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const;
    virtual DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const;
    virtual DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const;
    virtual bool restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const;
    virtual bool mkdir(const QSharedPointer<DFMMkdirEvent> &event) const;
    virtual bool touch(const QSharedPointer<DFMTouchFileEvent> &event) const;
    virtual bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const;

    virtual bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const;

    virtual const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const;
    virtual const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const;
    virtual const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const;

    virtual bool addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const;
    virtual bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const;

    virtual bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const;
    virtual bool shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const;
    virtual bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const;
    virtual bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const;

    virtual bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const;
    virtual bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const;
    virtual QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const;

    virtual DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const;
    virtual DFM_NAMESPACE::DFileDevice *createFileDevice(const QSharedPointer<DFMUrlBaseEvent> &event) const;
    virtual DFM_NAMESPACE::DFileHandler *createFileHandler(const QSharedPointer<DFMUrlBaseEvent> &event) const;
    virtual DFM_NAMESPACE::DStorageInfo *createStorageInfo(const QSharedPointer<DFMUrlBaseEvent> &event) const;

    virtual bool setExtraProperties(const QSharedPointer<DFMSetFileExtraProperties> &event) const;
};

#endif // ABSTRACTFILECONTROLLER_H
