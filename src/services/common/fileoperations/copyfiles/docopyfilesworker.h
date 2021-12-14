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
#include <QMultiMap>

class QWaitCondition;
DSC_BEGIN_NAMESPACE
USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
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
    bool doWork() override;
    void stop() override;
    void setStat(const AbstractJobHandler::JobState &stat) override;
    bool initArgs() override;
    void endWork() override;
    AbstractJobHandler::SupportActions supportActions(const AbstractJobHandler::JobErrorType &error) override;

protected:
    void determineCountProcessType();
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
    void emitSpeedUpdatedNotify(const qint64 &writSize);
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const QString &errorMsg = QString());
    QString formatFileName(const QString &fileName);
    QString getNonExistFileName(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer targetDir);
    qint64 getWriteDataSize();
    qint64 getTidWriteSize();
    qint64 getSectorsWritten();
    void readAheadSourceFile(const AbstractFileInfoPointer &fileInfo);
    void cancelThreadProcessingError();

private slots:
    void onUpdateProccess() override;

private:
    QAtomicInteger<qint64> skipWritSize { 0 };   // 当前写入文件的大小
    QAtomicInteger<qint64> currentWritSize { 0 };   // 跳过的文件大小

    QTime time;   // time eslape
    AbstractFileInfoPointer targetInfo { nullptr };   // target file infor pointer
    QSharedPointer<StorageInfo> targetStorageInfo { nullptr };   // target file's device infor
    long copyTid = { -1 };   // 使用 /pric/[pid]/task/[tid]/io 文件中的的 writeBytes 字段的值作为判断已写入数据的依据
    CountWriteSizeType countWriteType { CountWriteSizeType::kTidType };   // get write size type
    qint8 targetIsRemovable { 1 };   // 目标磁盘设备是不是可移除或者热插拔设备
    qint16 targetLogSecionSize { 512 };   // 目标设备逻辑扇区大小
    qint64 targetDeviceStartSectorsWritten { 0 };   // 记录任务开始时目标磁盘设备已写入扇区数
    QString targetSysDevPath;   // /sys/dev/block/x:x
    QSharedPointer<QQueue<QSharedPointer<SmallFileThreadCopyInfo>>> smallFileThreadCopyInfoQueue;   // copy small file thread information Queue
    QSharedPointer<QMutex> smallFileThreadCopyInfoQueueMutex { nullptr };   // copy small file thread information Queue's mutex
    QSharedPointer<QThreadPool> threadPool { nullptr };   // copy small file thread pool
    DThreadList<QSharedPointer<DirSetPermissonInfo>> dirPermissonList;   // dir set Permisson list
};

DSC_END_NAMESPACE

#endif   // DOCOPYFILESWORKER_H
