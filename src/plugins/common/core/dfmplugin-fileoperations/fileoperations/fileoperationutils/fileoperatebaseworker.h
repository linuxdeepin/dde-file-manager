/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             liyigang<liyigang@uniontech.com>
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
#ifndef FILEOPERATEBASEWORKER_H
#define FILEOPERATEBASEWORKER_H

#include "fileoperations/fileoperationutils/abstractworker.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QTime>

class QObject;

DPFILEOPERATIONS_BEGIN_NAMESPACE
class StorageInfo;
class DoCopyFileWorker;
using StoragePointer = QSharedPointer<StorageInfo>;
class FileOperateBaseWorker : public AbstractWorker, public QEnableSharedFromThis<AbstractFileInfo>
{

public:
    explicit FileOperateBaseWorker(QObject *parent = nullptr);
    virtual ~FileOperateBaseWorker() override;

    struct DirSetPermissonInfo
    {
        QFileDevice::Permissions permission;
        QUrl target;
    };

    using DirPermsissonPointer = QSharedPointer<DirSetPermissonInfo>;
    using DirPermissonList = DThreadList<DirPermsissonPointer>;

    struct SmallFileThreadCopyInfo
    {
        AbstractFileInfoPointer fromInfo { nullptr };
        AbstractFileInfoPointer toInfo { nullptr };
    };

public:
    bool doCheckFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QString &fileName,
                     AbstractFileInfoPointer &newTargetInfo, bool *skip);
    bool doCheckNewFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                        AbstractFileInfoPointer &newTargetInfo, QString &fileNewName,
                        bool *skip, bool isCountSize = false);
    bool checkDiskSpaceAvailable(const QUrl &fromUrl, const QUrl &toUrl,
                                 QSharedPointer<StorageInfo> targetStorageInfo, bool *skip);
    void setTargetPermissions(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo);
    void setAllDirPermisson();
    void determineCountProcessType();
    qint64 getWriteDataSize();
    qint64 getTidWriteSize();
    qint64 getSectorsWritten();
    void readAheadSourceFile(const AbstractFileInfoPointer &fileInfo);
    void syncFilesToDevice();
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error, const QString &errorMsg = QString());
    // notify
    void emitSpeedUpdatedNotify(const qint64 &writSize);

    bool deleteFile(const QUrl &fromUrl, const QUrl &toUrl, bool *result, const bool force = false);
    bool deleteDir(const QUrl &fromUrl, const QUrl &toUrl, bool *result, const bool force = false);
    bool copyFileFromTrash(const QUrl &urlSource, const QUrl &urlTarget, dfmio::DFile::CopyFlag flag);

    bool copyAndDeleteFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &targetPathInfo, const AbstractFileInfoPointer &toInfo,
                           bool *result);
    bool createSystemLink(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                          const bool followLink, const bool doCopy,
                          bool *result);
    bool canWriteFile(const QUrl &url) const;

    bool doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *skip);
    bool checkAndCopyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *skip);
    bool checkAndCopyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *skip);

private:
    void setSkipValue(bool *skip, AbstractJobHandler::SupportAction action);
    void initThreadCopy();
    void initSignalCopyWorker();
    bool createNewTargetInfo(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                             AbstractFileInfoPointer &newTargetInfo, const QUrl &fileNewUrl,
                             bool *skip, bool isCountSize = false);
    QUrl createNewTargetUrl(const AbstractFileInfoPointer &toInfo, QString &fileNewName);

protected Q_SLOTS:
    void emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                         const quint64 id, const QString &errorMsg = QString()) override;
    virtual void emitCurrentTaskNotify(const QUrl &from, const QUrl &to) override;

private:
    QVariant checkLinkAndSameUrl(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &newTargetInfo, const bool isCountSize);
    QVariant doActionReplace(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &newTargetInfo, const bool isCountSize);
    QVariant doActionMerge(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &newTargetInfo, const bool isCountSize);

protected:
    QTime time;   // time eslape
    AbstractFileInfoPointer targetInfo { nullptr };   // target file infor pointer
    StoragePointer targetStorageInfo { nullptr };   // target file's device infor
    CountWriteSizeType countWriteType { CountWriteSizeType::kTidType };   // get write size type
    long copyTid = { -1 };   // 使用 /pric/[pid]/task/[tid]/io 文件中的的 writeBytes 字段的值作为判断已写入数据的依据
    qint64 targetDeviceStartSectorsWritten { 0 };   // 记录任务开始时目标磁盘设备已写入扇区数
    QString targetSysDevPath;   // /sys/dev/block/x:x
    qint16 targetLogSecionSize { 512 };   // 目标设备逻辑扇区大小
    qint8 targetIsRemovable { 1 };   // 目标磁盘设备是不是可移除或者热插拔设备
    DirPermissonList dirPermissonList;   // dir set Permisson list

    std::atomic_int threadInfoVectorSize { 0 };
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATEBASEWORKER_H
