/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#ifndef MERGEDDESKTOPCONTROLLER_H
#define MERGEDDESKTOPCONTROLLER_H

#include "dabstractfilecontroller.h"

enum DMD_TYPES : unsigned int {
    DMD_PICTURE, DMD_MUSIC, DMD_APPLICATION, DMD_VIDEO, DMD_DOCUMENT, DMD_OTHER, DMD_FOLDER,
    DMD_FIRST_TYPE = DMD_PICTURE, DMD_ALL_TYPE = DMD_FOLDER, DMD_ALL_ENTRY = DMD_OTHER
};

class DFileWatcher;
class MergedDesktopController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit MergedDesktopController(QObject *parent = nullptr);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &) const override;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const override;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const override;

    bool mkdir(const QSharedPointer<DFMMkdirEvent> &event) const override; // AppController::actionNewFolder 有问题
    bool touch(const QSharedPointer<DFMTouchFileEvent> &event) const override;
    bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const override;
    bool compressFiles(const QSharedPointer<DFMCompressEvent> &event) const override;
    bool decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const override;
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const override;

    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const override;
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const override;
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const override;

    DFM_NAMESPACE::DFileDevice *createFileDevice(const QSharedPointer<DFMUrlBaseEvent> &event) const override;
    DFM_NAMESPACE::DFileHandler *createFileHandler(const QSharedPointer<DFMUrlBaseEvent> &event) const override;
    DFM_NAMESPACE::DStorageInfo *createStorageInfo(const QSharedPointer<DFMUrlBaseEvent> &event) const override;

    bool setExtraProperties(const QSharedPointer<DFMSetFileExtraProperties> &event) const override;

    const static QString entryNameByEnum(DMD_TYPES singleType);
    static DMD_TYPES entryTypeByName(QString entryName);
    static DUrl getVirtualEntryPath(DMD_TYPES oneType);
    static DUrl convertToDFMMDPath(const DUrl &oriUrl);
    static DUrl convertToDFMMDPath(const DUrl &oriUrl, DMD_TYPES oneType);
    static DUrl convertToRealPath(const DUrl &oneUrl);
    static DUrlList convertToRealPaths(DUrlList urlList);
    static DMD_TYPES checkUrlArrangedType(const DUrl url);

public slots:
    void desktopFilesCreated(const DUrl &url);
    void desktopFilesRemoved(const DUrl &url);
    void desktopFilesRenamed(const DUrl &oriUrl, const DUrl &dstUrl);

private:
    void initData() const;
    void appendEntryFiles(QList<DAbstractFileInfoPointer> &infoList, const DMD_TYPES &entryType) const;

    DFileWatcher* m_desktopFileWatcher;
    mutable DUrl currentUrl;
//    mutable bool dataInitialized = false;
    mutable QMap<DMD_TYPES, QList<DUrl> > arrangedFileUrls;
};

#endif // MERGEDDESKTOPCONTROLLER_H
