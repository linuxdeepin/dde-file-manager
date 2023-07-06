// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESTATISSTICSJOB_P_H
#define FILESTATISSTICSJOB_P_H

#include <dfm-base/utils/filestatisticsjob.h>
#include <dfm-base/interfaces/abstractdiriterator.h>

#include <QObject>

#include <fts.h>

namespace dfmbase {
class FileStatisticsJobPrivate : public QObject
{
public:
    explicit FileStatisticsJobPrivate(FileStatisticsJob *qq);
    ~FileStatisticsJobPrivate();

    void setState(FileStatisticsJob::State s);

    bool jobWait();
    bool stateCheck();

    void processFile(const QUrl &url, const bool followLink, QQueue<QUrl> &directoryQueue);
    void processFileByFts(const QUrl &url, const bool followLink);
    void emitSizeChanged();
    int countFileCount(const char *name);
    bool checkFileType(const FileInfo::FileType &fileType);
    FileInfo::FileType getFileType(const uint mode);
    void statisticDir(const QUrl &url, FTS *fts, const bool singleDepth, FTSENT *ent);
    void statisticFile(FTSENT *ent);
    void statisticSysLink(const QUrl &currentUrl, FTS *fts, FTSENT *ent, const bool singleDepth, const bool followLink);
    bool checkInode(const FileInfoPointer info);
    bool checkInode(FTSENT *ent, FTS *fts);

    FileStatisticsJob *q;
    QTimer *notifyDataTimer;

    QAtomicInt state = FileStatisticsJob::kStoppedState;
    FileStatisticsJob::FileHints fileHints;

    QList<QUrl> sourceUrlList;
    QWaitCondition waitCondition;
    QElapsedTimer elapsedTimer;

    QAtomicInteger<qint64> totalSize = { 0 };
    QAtomicInteger<qint64> totalProgressSize { 0 };
    QAtomicInt filesCount { 0 };
    QAtomicInt directoryCount { 0 };
    SizeInfoPointer sizeInfo { nullptr };
    QList<QUrl> fileStatistics;
    QList<QString> skipPath;
    QList<quint64> inodelist;
    AbstractDirIteratorPointer iterator { nullptr };
    std::atomic_bool iteratorCanStop { false };
};
}
#endif // FILESTATISSTICSJOB_P_H
