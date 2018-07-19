/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DFILECOPYMOVEJOB_P_H
#define DFILECOPYMOVEJOB_P_H

#include "dfilecopymovejob.h"
#include "dstorageinfo.h"

#include <QWaitCondition>
#include <QPointer>
#include <QStack>
#include <QElapsedTimer>

typedef QExplicitlySharedDataPointer<DAbstractFileInfo> DAbstractFileInfoPointer;

DFM_BEGIN_NAMESPACE

class DFileHandler;
class DFileStatisticsJob;
class ElapsedTimer;
class DFileCopyMoveJobPrivate
{
public:
    struct JobInfo {
        enum Type {
            Preprocess,
            Copy,
            Move,
            Remove,
            Link
        };

        Type action;
        QPair<DUrl, DUrl> targetUrl;
    };

    DFileCopyMoveJobPrivate(DFileCopyMoveJob *qq);
    ~DFileCopyMoveJobPrivate();

    static QString errorToString(DFileCopyMoveJob::Error error);

    void setState(DFileCopyMoveJob::State s);
    void setError(DFileCopyMoveJob::Error e, const QString &es = QString());
    void unsetError();
    DFileCopyMoveJob::Action handleError(const DAbstractFileInfo *sourceInfo, const DAbstractFileInfo *targetInfo);
    bool jobWait();
    bool stateCheck();
    bool checkFileSize(qint64 size) const;
    bool checkFreeSpace(qint64 needSize);
    QString formatFileName(const QString &name) const;

    static QString getNewFileName(const DAbstractFileInfo *sourceFileInfo, const DAbstractFileInfo *targetDirectory);

    bool doProcess(const DUrl &from, DAbstractFileInfoPointer source_info, const DAbstractFileInfo *target_info);
    bool mergeDirectory(DFileHandler *handler, const DAbstractFileInfo *fromInfo, const DAbstractFileInfo *toInfo);
    bool doCopyFile(const DAbstractFileInfo *fromInfo, const DAbstractFileInfo *toInfo, int blockSize = 1048576);
    bool doRemoveFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo);
    bool doRenameFile(DFileHandler *handler, const DAbstractFileInfo *oldInfo, const DAbstractFileInfo *newInfo);
    bool doLinkFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo, const QString &linkPath);

    bool process(const DUrl &from, const DAbstractFileInfo *target_info);
    bool copyFile(const DAbstractFileInfo *fromInfo, const DAbstractFileInfo *toInfo, int blockSize = 1048576);
    bool removeFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo);
    bool renameFile(DFileHandler *handler, const DAbstractFileInfo *oldInfo, const DAbstractFileInfo *newInfo);
    bool linkFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo, const QString &linkPath);

    void beginJob(JobInfo::Type type, const DUrl &from, const DUrl &target);
    void endJob();
    void joinToCompletedFileList(const DUrl &from, const DUrl &target, qint64 dataSize);
    void joinToCompletedDirectoryList(const DUrl &from, const DUrl &target, qint64 dataSize);
    void updateProgress();
    void updateSpeed();
    void _q_updateProgress();

    DFileCopyMoveJob *q_ptr;

    QWaitCondition waitCondition;

    DFileCopyMoveJob::Handle *handle = nullptr;
    DFileCopyMoveJob::Mode mode = DFileCopyMoveJob::CopyMode;
    DFileCopyMoveJob::Error error = DFileCopyMoveJob::NoError;
    DFileCopyMoveJob::FileHints fileHints = 0;
    QString errorString;
    QAtomicInt state = DFileCopyMoveJob::StoppedState;
    DFileCopyMoveJob::Action lastErrorHandleAction = DFileCopyMoveJob::NoAction;

    DUrlList sourceUrlList;
    DUrlList targetUrlList;
    DUrl targetUrl;
    DStorageInfo targetStorageInfo;
    QPointer<QThread> threadAtStart;
    DFileCopyMoveJob::Action actionOfError[DFileCopyMoveJob::UnknowError] = {DFileCopyMoveJob::NoAction};
    DFileStatisticsJob *fileStatistics = nullptr;

    QStack<JobInfo> jobStack;
    QList<QPair<DUrl, DUrl>> completedFileList;
    QList<QPair<DUrl, DUrl>> completedDirectoryList;
    int completedFilesCount = 0;
    qint64 completedDataSize = 0;
    QPair<qint64 /*total*/, qint64 /*writed*/> currentJobDataSizeInfo;
    int currentJobFileHandle = -1;
    ElapsedTimer *updateSpeedElapsedTimer = nullptr;
    QTimer *updateSpeedTimer = nullptr;
    int timeOutCount = 0;
    bool needUpdateProgress = false;

    Q_DECLARE_PUBLIC(DFileCopyMoveJob)
};

DFM_END_NAMESPACE

#endif // DFILECOPYMOVEJOB_P_H
