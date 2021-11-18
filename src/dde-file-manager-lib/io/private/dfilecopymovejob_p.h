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
#ifndef DFILECOPYMOVEJOB_P_H
#define DFILECOPYMOVEJOB_P_H

#include "dfilecopymovejob.h"
#include "dstorageinfo.h"

#include "private/qobject_p.h"
#include <QWaitCondition>
#include <QPointer>
#include <QStack>
#include <QElapsedTimer>
#include <QThreadPool>
#include <QMutex>
#include <QFuture>
#include <QQueue>
#include <QFileDevice>

#include <fcntl.h>

#include "dfiledevice.h"

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

    struct FileCopyInfo {
        bool closeflag;
        bool isdir;
        QSharedPointer<DFileHandler> handler;
        DAbstractFileInfoPointer frominfo;
        DAbstractFileInfoPointer toinfo;
        char *buffer;
        qint64 size;
        qint64 currentpos;
        QFileDevice::Permissions permission;
        FileCopyInfo() : closeflag(true)
            , isdir(false)
            , handler(nullptr)
            , frominfo(nullptr)
            , toinfo(nullptr)
            , buffer(nullptr)
            , size(0)
            , currentpos(0)
            , permission(QFileDevice::ReadOwner)
        {

        }
        FileCopyInfo(const FileCopyInfo &other) : closeflag(other.closeflag)
            , isdir(other.isdir)
            , handler(other.handler)
            , frominfo(other.frominfo)
            , toinfo(other.toinfo)
            , buffer(other.buffer)
            , size(other.size)
            , currentpos(other.currentpos)
            , permission(other.permission)
        {

        }
    };

    struct ThreadCopyInfo {
        QSharedPointer<DFileHandler> handler = nullptr;
        DAbstractFileInfoPointer fromInfo;
        DAbstractFileInfoPointer toInfo;
        QSharedPointer<DFileDevice> fromDevice = nullptr;
        QSharedPointer<DFileDevice> toDevice = nullptr;
    };

    struct DirSetPermissonInfo {
        QSharedPointer<DFileHandler> handler = nullptr;
        QFileDevice::Permissions permission;
        DUrl target;
    };

    typedef QSharedPointer<FileCopyInfo> FileCopyInfoPointer;

    explicit DFileCopyMoveJobPrivate(DFileCopyMoveJob *qq);
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
    DFileCopyMoveJob::Action handleError(const DAbstractFileInfoPointer sourceInfo, const DAbstractFileInfoPointer targetInfo);
    DFileCopyMoveJob::Action setAndhandleError(DFileCopyMoveJob::Error e, const DAbstractFileInfoPointer sourceInfo,
                                               const DAbstractFileInfoPointer targetInfo, const QString &es = QString());

    bool isRunning(); // bug 26333, add state function to check the job status
    bool jobWait();
    bool stateCheck();
    bool checkFileSize(qint64 size) const;
    bool checkFreeSpace(qint64 needSize);
    QString formatFileName(const QString &name) const;

    static QString getNewFileName(const DAbstractFileInfoPointer sourceFileInfo, const DAbstractFileInfoPointer targetDirectory);

    bool doProcess(const DUrl &from, const DAbstractFileInfoPointer source_info, const DAbstractFileInfoPointer target_info, const bool isNew = false);
    bool mergeDirectory(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo);
    bool doCopyFile(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize = 1048576);
    bool doCopySmallFilesOnDisk(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo,
                                const QSharedPointer<DFileDevice> &fromDevice, const QSharedPointer<DFileDevice> &toDevice,
                                const QSharedPointer<DFileHandler> &handler);
    //线程池中拷贝大量小文件
    bool doThreadPoolCopyFile();
    //拷贝文件到块设备（除光驱和系统所在的磁盘）
    bool doCopyFileOnBlock(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize = 1048576);
    bool doRemoveFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo,
                      const DAbstractFileInfoPointer &toInfo = DAbstractFileInfoPointer(nullptr));
    bool doRenameFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer oldInfo, const DAbstractFileInfoPointer newInfo);
    bool doLinkFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo, const QString &linkPath);

    /**
     * @brief convertTrashFile 对回收站根目录下的文件进行转换，将其移动到上级expunged目录下
     * @param fileInfo 文件信息
     */
    void convertTrashFile(DAbstractFileInfoPointer &fileInfo);

    bool process(const DUrl from, const DAbstractFileInfoPointer target_info);
    bool process(const DUrl from, const DAbstractFileInfoPointer source_info, const DAbstractFileInfoPointer target_info, const bool isNew = false);
    bool copyFile(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize = 1048576);
    bool removeFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo);
    bool renameFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer oldInfo, const DAbstractFileInfoPointer newInfo);
    bool linkFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo, const QString &linkPath);

    void beginJob(JobInfo::Type type, const DUrl from, const DUrl target, const bool isNew = false);
    void endJob(const bool isNew = false);
    void enterDirectory(const DUrl from, const DUrl to);
    void leaveDirectory();
    void joinToCompletedFileList(const DUrl from, const DUrl target, qint64 dataSize);
    void joinToCompletedDirectoryList(const DUrl from, const DUrl target, qint64 dataSize);
    void updateProgress();
    void updateCopyProgress();
    void updateMoveProgress();
    void updateSpeed();
    void _q_updateProgress();
    void countrefinesize(const qint64 &size);

    //第二版优化
    bool writeRefineThread();
    bool writeToFileByQueue();
    bool skipReadFileDealWriteThread(const DUrl &url);
    void cancelReadFileDealWriteThread();
    void setRefineCopyProccessSate(const DFileCopyMoveJob::RefineCopyProccessSate &stat);
    bool checkRefineCopyProccessSate(const DFileCopyMoveJob::RefineCopyProccessSate &stat);
    void checkTagetNeedSync();//检测目标目录是网络文件就每次拷贝去同步，否则网络很卡时会因为同步卡死
    void checkTagetIsFromBlockDevice();//检查目标文件是否是块设备
    bool checkWritQueueEmpty();
    bool checkWritQueueCount();
    QSharedPointer<FileCopyInfo> writeQueueDequeue();
    void writeQueueEnqueue(const QSharedPointer<FileCopyInfo> &copyinfo);
    //错误队列处理
    void errorQueueHandling();
    //当前错误队列处理完成
    void errorQueueHandled(const bool &isNotCancel = true);
    //清理当前拷贝信息
    void releaseCopyInfo(const FileCopyInfoPointer &info);
    /**
     * @brief setCutTrashData    保存剪切回收站文件路径
     * @param fileNameList       文件路径
     */
    void setCutTrashData(QVariant fileNameList);
    //fix bug 61565
    //线程安全保存当前工作的device
    void saveCurrentDevice(const DUrl &url,const QSharedPointer<DFileDevice> device);
    //线程安全移出当前工作的device
    void removeCurrentDevice(const DUrl &url);
    //线程安全移出当前工作的device
    void stopAllDeviceOperation();
    //清理线程池资源
    void clearThreadPool();

    void waitRefineThreadFinish();
    void setLastErrorAction(const DFileCopyMoveJob::Action &action);
    DFileCopyMoveJob::Action getLastErrorAction();
    qint64 reopenGvfsFiles(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo,
                          QSharedPointer<DFileDevice> &fromDevice, QSharedPointer<DFileDevice> &toDevice,
                          const bool &isWriteError = true);
    DFileCopyMoveJob::Action seekFile(const DAbstractFileInfoPointer &fileInfo,
                                          QSharedPointer<DFileDevice> &device, const qint64 &pos);
    DFileCopyMoveJob::Action openGvfsFile(const DAbstractFileInfoPointer &fileInfo,
                                          QSharedPointer<DFileDevice> &device,
                                          const QIODevice::OpenMode &flags);
    void cleanCopySources(char *data, const QSharedPointer<DFileDevice> &fromDevice,
                          const QSharedPointer<DFileDevice> &toDevice, bool &isError);
    DFileCopyMoveJob::GvfsRetryType gvfsFileRetry(char * data, bool &isErrorOccur, qint64 &currentPos, const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo,
                                                  QSharedPointer<DFileDevice> &fromDevice, QSharedPointer<DFileDevice> &toDevice,
                                                  const bool &isWriteError = true);
    void readAheadSourceFile(const DAbstractFileInfoPointer &fromInfo);
    bool handleUnknowUrlError(const DAbstractFileInfoPointer &fromInfo,const DAbstractFileInfoPointer &toInfo);
    bool handleUnknowError(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo, const QString &errorStr);
    void sendCopyInfo(const DAbstractFileInfoPointer &fromInfo,const DAbstractFileInfoPointer &toInfo);
    void cleanDoCopyFileSource(char *data, const DAbstractFileInfoPointer &fromInfo,const DAbstractFileInfoPointer &toInfo, const QSharedPointer<DFileDevice> &fromDevice,
                               const QSharedPointer<DFileDevice> &toDevice);
    // 初始化优化状态
    void initRefineState();
    //! 剪切回收站文件路径
    QQueue<QString> m_fileNameList;

    DFileCopyMoveJob *q_ptr;

    QWaitCondition waitCondition;
    QWaitCondition m_waitConditionCopyLargeFileOnDisk;

    DFileCopyMoveJob::Handle *handle = nullptr;
    DFileCopyMoveJob::Mode mode = DFileCopyMoveJob::CopyMode;
    DFileCopyMoveJob::Error error = DFileCopyMoveJob::NoError;
    DFileCopyMoveJob::FileHints fileHints = DFileCopyMoveJob::NoHint;
    QString errorString;
    QAtomicInt state = DFileCopyMoveJob::StoppedState;
    QMap<QThread *, DFileCopyMoveJob::Action> m_lastErrorHandleAction;
    QMutex m_lastErrorHandleActionMutex;

    DUrlList sourceUrlList;
    DUrlList targetUrlList;
    DUrl targetUrl;


    qint64 totalsize = 0;
    qint32 totalfilecount = 0;
    QAtomicInteger<bool> m_isCountSizeOver = false;
    QAtomicInteger<bool> cansetnoerror = true;
    QAtomicInteger<bool> m_isFileOnDiskUrls = false;

    qint64 m_tatol = 0;
    qint64 m_sart = 0;

    QQueue<FileCopyInfoPointer> m_writeFileQueue;
    QAtomicInt m_copyRefineFlag = DFileCopyMoveJob::NoProccess;
    QFuture<void> m_writeResult, m_syncResult;


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
    //跳过文件大小统计
    qint64 skipFileSize = 0;
    // 已经写入到block设备的总大小
    qint64 completedDataSizeOnBlockDevice = 0;
    QPair<qint64 /*total*/, qint64 /*writed*/> currentJobDataSizeInfo;
    int currentJobFileHandle = -1;
    ElapsedTimer *updateSpeedElapsedTimer = nullptr;
    QTimer *updateSpeedTimer = nullptr;
    int timeOutCount = 0;
    QAtomicInteger<bool> needUpdateProgress = false;
    QAtomicInteger<bool> countStatisticsFinished = false;
    // 线程id
    long tid = -1;

    qreal lastProgress = 0.01; // 上次刷新的进度

    int currentThread = 0;

    QAtomicInteger<bool> m_bTaskDailogClose = false;

    QAtomicInt m_refineStat = DFileCopyMoveJob::RefineLocal;
    //优化盘内拷贝，启用的线程池
    QThreadPool m_pool;
    QAtomicInteger<bool> m_bDestLocal = false;
    qint64 m_refineCopySize = 0;
    QMutex m_refineMutex;
    //是否需要显示进度条
    QAtomicInteger<bool> m_isNeedShowProgress = false;
    //是否需要每读写一次同步
    bool m_isEveryReadAndWritesSnc = false;
    QAtomicInteger<bool> m_isVfat = false;
    QAtomicInt m_openFlag = O_CREAT | O_WRONLY | O_TRUNC;
    //分断拷贝的线程数量
    QAtomicInt m_bigFileThreadCount = 0;
    QAtomicInteger<bool> m_isWriteThreadStart = false;
    //目标目录是否是来自块设备
    QAtomicInteger<bool> m_isTagFromBlockDevice = false;
    //读线程跳过的文件
    QQueue<DUrl> m_skipFileQueue;
    //目标文件是否是gvfs目录
    QAtomicInteger<bool> m_isTagGvfsFile = false;
    //拷贝信息的队列锁
    QMutex m_copyInfoQueueMutex;
    QMutex m_skipFileQueueMutex;
    //当前拷贝的device
    QMap<DUrl,QSharedPointer<DFileDevice>> m_currentDevice;
    QMutex m_currentDeviceMutex;
    //当前本地目录大小
    qint32 m_currentDirSize = 0;
    //当前拷贝进程是否显示了进度条
    QAtomicInteger<bool> m_isProgressShow = false;

    QMutex m_checkStatMutex;

    //拷贝本地大文件中
    QAtomicInteger<bool> m_isCopyLargeFileOnDiskWait = false;
    static QQueue<DFileCopyMoveJob*> CopyLargeFileOnDiskQueue;
    static QMutex CopyLargeFileOnDiskMutex;

    //多线程拷贝小文件错误处理变量
    QQueue<Qt::HANDLE> m_errorQueue;
    QWaitCondition m_errorCondition;
    QMutex m_errorQueueMutex;
    QMutex m_stopMutex;
    QMutex m_clearThreadPoolMutex;
    QQueue<QSharedPointer<ThreadCopyInfo>> m_threadInfos;
    QMutex m_threadMutex;
    QMap<DUrl,DUrl> m_emitUrl;
    QMutex m_emitUrlMutex;

    //打开写入文件的fd
    QMap<DUrl,int> m_writeOpenFd;
    QList<QSharedPointer<DirSetPermissonInfo>> m_dirPermissonList;

    qint64 m_gvfsFileInnvliadProgress = 0;

    Q_DECLARE_PUBLIC(DFileCopyMoveJob)
};

DFM_END_NAMESPACE

#endif // DFILECOPYMOVEJOB_P_H
