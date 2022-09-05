// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
