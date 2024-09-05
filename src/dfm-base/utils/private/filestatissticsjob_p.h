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
    void processFile(const FileInfoPointer &info, const bool followLink, QQueue<QUrl> &directoryQueue);
    void emitSizeChanged();
    int countFileCount(const char *name);
    bool checkFileType(const FileInfo::FileType &fileType);
    bool checkInode(const FileInfoPointer info);

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
    QSet<QUrl> fileStatistics;
    QSet<QUrl> allFiles;
    QSet<QString> skipPath;
    QSet<quint64> inodelist;
    AbstractDirIteratorPointer iterator { nullptr };
    std::atomic_bool iteratorCanStop { false };
};
}
#endif // FILESTATISSTICSJOB_P_H
