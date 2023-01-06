// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTCONTROLLER_H
#define RECENTCONTROLLER_H

#include "dabstractfilecontroller.h"
#include "models/recentfileinfo.h"

#include <QWaitCondition>
#include <QMutex>

class QFileSystemWatcher;
class DAbstractFileInfo;
class DFileWatcher;
class QTimer;

class RecentController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit RecentController(QObject *parent = nullptr);

    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const override;
    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const override;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const override;
    bool openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;
    bool compressFiles(const QSharedPointer<DFMCompressEvent> &event) const override;
    bool decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const override;
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const override;

    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;

    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const override;
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const override;
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const override;

    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

public:
    mutable QMap<DUrl, RecentPointer> recentNodes;

private:
    static DUrlList realUrlList(const DUrlList &recentUrls);
    void handleFileChanged();
    void asyncHandleFileChanged();

private:
    QString m_xbelPath;
    QMap<QString, QString> m_bindPathMaps;
    DFileWatcher *m_watcher;
    QWaitCondition m_condition;
    QMutex m_xbelFileLock;
};

#endif // RECENTCONTROLLER_H
