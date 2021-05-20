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

#ifndef TAGCONTROLLER_H
#define TAGCONTROLLER_H


#include "dabstractfilecontroller.h"

class TagController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit TagController(QObject* const parent = nullptr);
    ~TagController()=default;

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent>& event) const override;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const override;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const override;
    bool openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const override;
    bool compressFiles(const QSharedPointer<DFMCompressEvent> &event) const override;
    bool decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const override;
    bool decompressFileHere(const QSharedPointer<DFMDecompressEvent> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const override;

    bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const override;

    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const override;
    bool addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const override;
    bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const override;
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const override;
    bool shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const override;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const override;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const override;

    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const override;
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const override;
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const override;
};


#endif // TAGCONTROLLER_H
