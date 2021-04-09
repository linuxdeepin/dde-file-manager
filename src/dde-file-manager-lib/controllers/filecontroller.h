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

#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include "dabstractfilecontroller.h"

class FileInfoGatherer;
class IconProvider;
class RecentHistoryManager;
class DAbstractFileInfo;
class FileMonitor;

class FileController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = nullptr);

    static bool findExecutable(const QString &executableName, const QStringList &paths = QStringList());

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;

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
    bool mkdir(const QSharedPointer<DFMMkdirEvent> &event) const override;
    bool touch(const QSharedPointer<DFMTouchFileEvent> &event) const override;

    bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const override;

    bool shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const override;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const override;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const override;

    bool addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const override;
    bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const override;
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const override;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;
    DFM_NAMESPACE::DFileDevice *createFileDevice(const QSharedPointer<DFMUrlBaseEvent> &event) const override;
    DFM_NAMESPACE::DFileHandler *createFileHandler(const QSharedPointer<DFMUrlBaseEvent> &event) const override;
    DFM_NAMESPACE::DStorageInfo *createStorageInfo(const QSharedPointer<DFMUrlBaseEvent> &event) const override;

    static bool customHiddenFileMatch(const QString &absolutePath, const QString &fileName);
    static bool privateFileMatch(const QString &absolutePath, const QString &fileName);

    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const override;
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const override;
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const override;

    bool renameFileByGio(const DUrl &oldUrl, const DUrl &newUrl) const;
private:
    QString checkDuplicateName(const QString &name) const;
    //修改复制拷贝流程，拷贝线程不去阻塞主线程，拷贝线程自己去处理，主线程直接返回，拷贝线程结束了在去处理以前的后续操作，delete还是走老流程
    DUrlList pasteFilesV2(const QSharedPointer<DFMPasteEvent> &event, DFMGlobal::ClipboardAction action, const DUrlList &list, const DUrl &target, bool slient = false, bool force = false, bool bold = false) const;
    //处理复制、粘贴和剪切(拷贝)结束后操作 fix bug 35855
    void dealpasteEnd(const DUrlList &lsit, const QSharedPointer<DFMPasteEvent> &event) const;
    bool isExtDeviceJobCase(void *curJob, const DUrl &url) const;
    bool isDiscburnJobCase(void *curJob, const DUrl &url) const;
    bool fileAdded(const DUrl &url) const;

    DUrl handleTagFileUrl(const DUrl &url) const;
};

#endif // FILECONTROLLER_H
