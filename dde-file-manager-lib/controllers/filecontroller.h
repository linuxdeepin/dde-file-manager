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
    explicit FileController(QObject *parent = 0);

    static bool findExecutable(const QString & executableName, const QStringList & paths = QStringList());

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    bool openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const Q_DECL_OVERRIDE;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool compressFiles(const QSharedPointer<DFMCompressEvent> &event) const Q_DECL_OVERRIDE;
    bool decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const Q_DECL_OVERRIDE;
    bool decompressFileHere(const QSharedPointer<DFMDecompressEvent> &event) const Q_DECL_OVERRIDE;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const Q_DECL_OVERRIDE;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const Q_DECL_OVERRIDE;
    bool mkdir(const QSharedPointer<DFMMkdirEvent> &event) const Q_DECL_OVERRIDE;
    bool touch(const QSharedPointer<DFMTouchFileEvent> &event) const Q_DECL_OVERRIDE;

    bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const override;

    bool shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const Q_DECL_OVERRIDE;

    bool addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const override;
    bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const override;
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;
    DFM_NAMESPACE::DFileDevice *createFileDevice(const QSharedPointer<DFMUrlBaseEvent> &event) const override;
    DFM_NAMESPACE::DFileHandler *createFileHandler(const QSharedPointer<DFMUrlBaseEvent> &event) const override;
    DFM_NAMESPACE::DStorageInfo *createStorageInfo(const QSharedPointer<DFMUrlBaseEvent> &event) const override;

    static bool customHiddenFileMatch(const QString &absolutePath, const QString &fileName);
    static bool privateFileMatch(const QString &absolutePath, const QString &fileName);

    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const Q_DECL_OVERRIDE;
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const Q_DECL_OVERRIDE;
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const Q_DECL_OVERRIDE;

private:
    QString checkDuplicateName(const QString &name) const;
    //修改复制拷贝流程，拷贝线程不去阻塞主线程，拷贝线程自己去处理，主线程直接返回，拷贝线程结束了在去处理以前的后续操作，delete还是走老流程
    DUrlList pasteFilesV2(const QSharedPointer<DFMPasteEvent> &event, DFMGlobal::ClipboardAction action,
                          const DUrlList &list, const DUrl &target, bool slient = false,
                          bool force = false, bool bold = false) const;
    //处理复制、粘贴和剪切(拷贝)结束后操作 fix bug 35855
    void dealpasteEnd(const DUrlList &lsit, const QSharedPointer<DFMPasteEvent> &event) const;
    bool isExtDeviceJobCase(void* curJob, const DUrl& url) const;
    bool isDiscburnJobCase(void* curJob, const DUrl& url) const;
};

#endif // FILECONTROLLER_H
