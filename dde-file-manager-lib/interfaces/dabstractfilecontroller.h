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

#ifndef ABSTRACTFILECONTROLLER_H
#define ABSTRACTFILECONTROLLER_H

#include "dabstractfileinfo.h"
#include "ddiriterator.h"

#include <QObject>
#include <QDir>
#include <QDirIterator>

class DFMEvent;
class DFMOpenFileEvent;
class DFMOpenFileByAppEvent;
class DFMCompressEvnet;
class DFMDecompressEvnet;
class DFMWriteUrlsToClipboardEvent;
class DFMRenameEvent;
class DFMDeleteEvent;
class DFMMoveToTrashEvent;
class DFMRestoreFromTrashEvent;
class DFMPasteEvent;
class DFMMkdirEvent;
class DFMTouchFileEvent;
class DFMOpenFileLocation;
class DFMCreateSymlinkEvent;
class DFMGetChildrensEvent;
class DFMCreateDiriterator;
class DFMCreateFileInfoEvnet;
class DFMCreateFileWatcherEvent;
class DFMOpenInTerminalEvent;
class DFMFileShareEvnet;
class DFMCancelFileShareEvent;
class DUrl;
class DAbstractFileWatcher;
class DFMSetFileTagsEvent;
class DFMRemoveTagsOfFileEvent;
class DFMGetTagsThroughFilesEvent;

typedef QList<DUrl> DUrlList;
class DAbstractFileController : public QObject
{
    Q_OBJECT

public:
    explicit DAbstractFileController(QObject *parent = 0);

    virtual bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const;
    virtual bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const;
    virtual bool compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const;
    virtual bool decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const;
    virtual bool decompressFileHere(const QSharedPointer<DFMDecompressEvnet> &event) const;
    virtual bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const;
    virtual bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const;
    virtual bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const;
    virtual DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const;
    virtual DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const;
    virtual bool restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const;
    virtual bool mkdir(const QSharedPointer<DFMMkdirEvent> &event) const;
    virtual bool touch(const QSharedPointer<DFMTouchFileEvent> &event) const;

    virtual bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const;

    virtual const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const;
    virtual const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const;
    virtual const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const;

    virtual bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const;
    virtual bool shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const;
    virtual bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const;
    virtual bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const;

    virtual bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const;
    virtual bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const;
    virtual QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const;

    virtual DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const;
};

#endif // ABSTRACTFILECONTROLLER_H
