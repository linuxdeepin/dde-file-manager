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

#ifndef TRASHMANAGER_H
#define TRASHMANAGER_H

#include "dabstractfilecontroller.h"

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QFileInfo>

class DAbstractFileInfo;
class FileMonitor;
class DFileWatcher;

class TrashManager : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit TrashManager(QObject *parent = nullptr);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    //! 多文件打开入口
    bool openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const override;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;
    /**
     * @brief sortByOriginPath 根据原始路径对list进行排序
     * @param list
     */
    void sortByOriginPath(DUrlList &list) const;
    bool restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    static bool restoreTrashFile(const DUrlList &list, DUrlList *restoreOriginUrls = nullptr);
    void cleanTrash(const QObject *sender = nullptr, bool silent = false) const;

    static bool isEmpty();
    static bool isWorking();
public slots:
    void trashFilesChanged(const DUrl &url);
private:
    bool m_isTrashEmpty;
    DFileWatcher *m_trashFileWatcher;
};

#endif // TRASHMANAGER_H
