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

#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "dabstractfilecontroller.h"

#include <QSet>
#include <QPair>

class SearchFileWatcher;
class SearchController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit SearchController(QObject *parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const Q_DECL_OVERRIDE;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const Q_DECL_OVERRIDE;
    bool restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const Q_DECL_OVERRIDE;

    bool compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const Q_DECL_OVERRIDE;
    bool decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const Q_DECL_OVERRIDE;

    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const Q_DECL_OVERRIDE;

    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const Q_DECL_OVERRIDE;

    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

private:
    static DUrl realUrl(const DUrl &searchUrl);
    static DUrlList realUrlList(const DUrlList &searchUrls);

    friend class SearchDiriterator;
    friend class SearchFileWatcher;
};

#endif // SEARCHCONTROLLER_H
