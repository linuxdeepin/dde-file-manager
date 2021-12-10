/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DOCOPYFILESWORKER_H
#define DOCOPYFILESWORKER_H

#include "dfm_common_service_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "dfm-base/utils/threadcontainer.hpp"
#include "dfm-base/file/local/localfilehandler.h"

#include <dfm-io/core/dfile.h>

#include <QObject>
#include <QMutex>
#include <QTime>
#include <QQueue>
#include <QThread>
#include <QThreadPool>

class QWaitCondition;
DSC_BEGIN_NAMESPACE
USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
class UpdateProccessTimer : public QObject
{
    Q_OBJECT
    friend class DoCopyFilesWorker;
    explicit UpdateProccessTimer(QObject *parent = nullptr)
        : QObject(parent) {}
signals:
    void updateProccessNotify();
private slots:
    void doStartTime()
    {
        emit updateProccessNotify();
        QThread::msleep(500);
    }

public:
    ~UpdateProccessTimer() = default;
};
class StorageInfo;
class DoCopyFilesWorker : public AbstractWorker
{
    friend class CopyFiles;
    Q_OBJECT
    explicit DoCopyFilesWorker(QObject *parent = nullptr);

    struct DirSetPermissonInfo
    {
        QFileDevice::Permissions permission;
        QUrl target;
    };

    struct SmallFileThreadCopyInfo
    {
        AbstractFileInfoPointer fromInfo { nullptr };
        AbstractFileInfoPointer toInfo { nullptr };
    };

public:
    enum class CountWriteSizeType : quint8 {
        kTidType,   // Read thread IO write size 使用 /pric/[pid]/task/[tid]/io 文件中的的 writeBytes 字段的值作为判断已写入数据的依据
        kWriteBlockType,   // Read write block device write block size
        kCustomizeType
    };
    virtual ~DoCopyFilesWorker() override;

protected:
    void doWork() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool initArgs();
    void statisticsFilesSize();
    void setCountProccessType();
    void startCountProccess();
    bool copyFiles();
    bool doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo);
    bool doCheckFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, AbstractFileInfoPointer &newTargetInfo);

    bool doCheckNewFile(const AbstractFileInfoPointer &fromInfo,
                        const AbstractFileInfoPointer &toInfo,
                        AbstractFileInfoPointer &newTargetInfo,
                        QString &fileNewName,
                        bool isCountSize = false);
    bool doCheckFileFreeSpace(const qint64 &size);
    bool creatSystemLink(const AbstractFileInfoPointer &fromInfo,
                         const AbstractFileInfoPointer &toInfo);
    bool checkAndCopyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo);
    bool doThreadPoolCopyFile();
    bool doCopyOneFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo);
    bool createFileDevices(const AbstractFileInfoPointer &fromInfo,
                           const AbstractFileInfoPointer &toInfo,
                           QSharedPointer<DFile> &fromeFile,
                           QSharedPointer<DFile> &toFile,
                           bool &result);
    bool createFileDevice(const AbstractFileInfoPointer &fromInfo,
                          const AbstractFileInfoPointer &toInfo,
                          const AbstractFileInfoPointer &needOpenInfo,
                          QSharedPointer<DFile> &file, bool &result);
    bool openFiles(const AbstractFileInfoPointer &fromInfo,
                   const AbstractFileInfoPointer &toInfo,
                   const QSharedPointer<DFile> &fromeFile,
                   const QSharedPointer<DFile> &toFile,
                   bool &result);
    bool openFile(const AbstractFileInfoPointer &fromInfo,
                  const AbstractFileInfoPointer &toInfo,
                  const QSharedPointer<DFile> &file,
                  const DFMIO::DFile::OpenFlag &flags,
                  bool &result);
    bool resizeTargetFile(const AbstractFileInfoPointer &fromInfo,
                          const AbstractFileInfoPointer &toInfo,
                          const QSharedPointer<DFile> &file,
                          bool &result);
    bool doReadFile(const AbstractFileInfoPointer &fromInfo,
                    const AbstractFileInfoPointer &toInfo,
                    const QSharedPointer<DFile> &fromDevice, char *data,
                    const qint64 &blockSize,
                    qint64 &readSize,
                    bool &result);
    bool doWriteFile(const AbstractFileInfoPointer &fromInfo,
                     const AbstractFileInfoPointer &toInfo,
                     const QSharedPointer<DFile> &toDevice,
                     const char *data,
                     const qint64 &readSize,
                     bool &result);
    bool checkAndcopyDir(const AbstractFileInfoPointer &fromInfo,
                         const AbstractFileInfoPointer &toInfo);
    void setTargetPermissions(const AbstractFileInfoPointer &fromInfo,
                              const AbstractFileInfoPointer &toInfo);
    bool verifyFileIntegrity(const qint64 &blockSize,
                             const ulong &sourceCheckSum,
                             const AbstractFileInfoPointer &fromInfo,
                             const AbstractFileInfoPointer &toInfo,
                             QSharedPointer<DFile> &toFile);
    void setAllDirPermisson();
    void syncFilesToDevice();

    void endCopy();
    void emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                         const QString &errorMsg = QString());
    void emitProccessChangedNotify(const qint64 &writSize);
    void emitStateChangedNotify();
    void emitCurrentTaskNotify(const QUrl &from, const QUrl &to);
    void emitSpeedUpdatedNotify(const qint64 &writSize);
    bool stateCheck();
    bool copyWait();
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const QString &errorMsg = QString());
    void setStat(const AbstractJobHandler::JobState &stat);
    AbstractJobHandler::SupportActions supportActions(const AbstractJobHandler::JobErrorType &error);
    QString errorToString(const AbstractJobHandler::JobErrorType &error);
    JobInfoPointer createCopyJobInfo(const QUrl &from, const QUrl &to);
    QString formatFileName(const QString &fileName);
    QString getNonExistFileName(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer targetDir);
    qint64 getWriteDataSize();
    qint64 getTidWriteSize();
    qint64 getSectorsWritten();
    void readAheadSourceFile(const AbstractFileInfoPointer &fileInfo);
    void cancelThreadProcessingError();
signals:   // update proccess timer use
    void startUpdateProccessTimer();
private slots:
    void onStatisticsFilesSizeFinish(const SizeInfoPoiter sizeInfo);
    void onUpdateProccess();

private:
    QAtomicInteger<qint64> sourceFilesTotalSize { 0 };   // 源文件的总大小
    QAtomicInteger<qint64> skipWritSize { 0 };   // 当前写入文件的大小
    QAtomicInteger<qint64> currentWritSize { 0 };   // 跳过的文件大小
    QSharedPointer<QList<QUrl>> allFilesList;   // 所有源文件的统计文件
    quint16 dirSize { 0 };   // 目录大小
    bool isSourceFileLocal { false };   // 源文件是否在可以出设备上
    bool isTargetFileLocal { false };   // 目标文件是否在可以出设备上
    QSharedPointer<QWaitCondition> waitCondition;   // 线程等待
    QMutex conditionMutex;   // 线程等待锁
    AbstractJobHandler::JobState currentState = AbstractJobHandler::JobState::kUnknowState;   // current state
    QTime time;   // time eslape
    AbstractFileInfoPointer targetInfo { nullptr };   // target file infor pointer
    QSharedPointer<LocalFileHandler> handler;   // file base operations handler
    QSharedPointer<StorageInfo> targetStorageInfo { nullptr };   // target file's device infor
    QSharedPointer<UpdateProccessTimer> updateProccessTimer { nullptr };   // update proccess timer
    QThread updateProccessThread;   // update proccess timer thread
    long copyTid = { -1 };   // 使用 /pric/[pid]/task/[tid]/io 文件中的的 writeBytes 字段的值作为判断已写入数据的依据
    CountWriteSizeType countWriteType { CountWriteSizeType::kTidType };   // get write size type
    qint8 targetIsRemovable { 1 };   // 目标磁盘设备是不是可移除或者热插拔设备
    qint16 targetLogSecionSize { 512 };   // 目标设备逻辑扇区大小
    qint64 targetDeviceStartSectorsWritten { 0 };   // 记录任务开始时目标磁盘设备已写入扇区数
    QString targetSysDevPath;   // /sys/dev/block/x:x
    QQueue<QSharedPointer<SmallFileThreadCopyInfo>> smallFileThreadCopyInfoQueue;   // copy small file thread information Queue
    QMutex smallFileThreadCopyInfoQueueMutex;   // copy small file thread information Queue's mutex
    QThreadPool threadPool;   // copy small file thread pool
    DThreadList<QSharedPointer<DirSetPermissonInfo>> dirPermissonList;   // dir set Permisson list
    QQueue<Qt::HANDLE> errorThreadIdQueue;   // Thread queue for processing errors
    QSharedPointer<QWaitCondition> errorCondition;   //  Condition variables that block other bad threads
    QMutex errorThreadIdQueueMutex;   // Condition variables that block other bad threads mutex
};

DSC_END_NAMESPACE

#endif   // DOCOPYFILESWORKER_H
