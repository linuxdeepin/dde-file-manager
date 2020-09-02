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

    struct DirectoryInfo {
        DStorageInfo sourceStorageInfo;
        DStorageInfo targetStorageInfo;
        QPair<DUrl, DUrl> url;
    };

    DFileCopyMoveJobPrivate(DFileCopyMoveJob *qq);
    ~DFileCopyMoveJobPrivate();

    static QString errorToString(DFileCopyMoveJob::Error error);
    // 返回当前线程已经往block设备写入的数据，返回的是本次和上次间隔时间内写入的大小
    static qint64 getWriteBytes(long tid);
    qint64 getWriteBytes() const;
    // 返回当前目标设备已写入扇区总数
    // /sys/dev/block/[x:x]/stat 的第7个字段
    // https://www.kernel.org/doc/Documentation/iostats.txt
    qint64 getSectorsWritten() const;
    // 返回已写入数据大小，根据多种情况，用不同的方式获取此数据
    qint64 getCompletedDataSize() const;

    void setState(DFileCopyMoveJob::State s);
    void setError(DFileCopyMoveJob::Error e, const QString &es = QString());
    void unsetError();
    DFileCopyMoveJob::Action handleError(const DAbstractFileInfo *sourceInfo, const DAbstractFileInfo *targetInfo);

    bool isRunning(); // bug 26333, add state function to check the job status
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
    bool process(const DUrl &from, const DAbstractFileInfoPointer &source_info, const DAbstractFileInfo *target_info);
    bool copyFile(const DAbstractFileInfo *fromInfo, const DAbstractFileInfo *toInfo, int blockSize = 1048576);
    bool removeFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo);
    bool renameFile(DFileHandler *handler, const DAbstractFileInfo *oldInfo, const DAbstractFileInfo *newInfo);
    bool linkFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo, const QString &linkPath);

    void beginJob(JobInfo::Type type, const DUrl &from, const DUrl &target);
    void endJob();
    void enterDirectory(const DUrl &from, const DUrl &to);
    void leaveDirectory();
    void joinToCompletedFileList(const DUrl &from, const DUrl &target, qint64 dataSize);
    void joinToCompletedDirectoryList(const DUrl &from, const DUrl &target, qint64 dataSize);
    void updateProgress();
    void updateCopyProgress();
    void updateMoveProgress();
    void updateSpeed();
    void _q_updateProgress();
    void checkTagetNeedSync();//检测目标目录是网络文件就每次拷贝去同步，否则网络很卡时会因为同步卡死

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


    // 是否可以使用 /pric/[pid]/task/[tid]/io 文件中的的 writeBytes 字段的值作为判断已写入数据的依据
    qint8 canUseWriteBytes : 1;
    // 目标磁盘设备是不是可移除或者热插拔设备
    qint8 targetIsRemovable : 1;
    // 逻辑扇区大小
    qint16 targetLogSecionSize = 512;
    // 记录任务开始时目标磁盘设备已写入扇区数
    qint64 targetDeviceStartSectorsWritten;
    // /sys/dev/block/x:x
    QString targetSysDevPath;
    // 目标设备所挂载的根目录
    QString targetRootPath;

    QPointer<QThread> threadOfErrorHandle;
    DFileCopyMoveJob::Action actionOfError[DFileCopyMoveJob::UnknowError] = {DFileCopyMoveJob::NoAction};
    DFileStatisticsJob *fileStatistics = nullptr;

    QStack<JobInfo> jobStack;
    QStack<DirectoryInfo> directoryStack;
    QList<QPair<DUrl, DUrl>> completedFileList;
    QList<QPair<DUrl, DUrl>> completedDirectoryList;
    int completedFilesCount = 0;
    int totalMoveFilesCount = 1;
    qint64 completedDataSize = 0;
    qint64 completedProgressDataSize = 0;
    // 已经写入到block设备的总大小
    qint64 completedDataSizeOnBlockDevice = 0;
    QPair<qint64 /*total*/, qint64 /*writed*/> currentJobDataSizeInfo;
    int currentJobFileHandle = -1;
    ElapsedTimer *updateSpeedElapsedTimer = nullptr;
    QTimer *updateSpeedTimer = nullptr;
    int timeOutCount = 0;
    bool needUpdateProgress = false;
    bool countStatisticsFinished = false;
    // 线程id
    long tid = -1;

    qreal lastProgress = 0.01; // 上次刷新的进度

    //是否需要每读写一次同步
    bool iseveryreadandwritesync = false;

    bool btaskdailogclose = false;

    Q_DECLARE_PUBLIC(DFileCopyMoveJob)
};

DFM_END_NAMESPACE

#endif // DFILECOPYMOVEJOB_P_H
