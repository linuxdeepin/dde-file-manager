// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATEBASEWORKER_H
#define FILEOPERATEBASEWORKER_H

#include "fileoperations/fileoperationutils/abstractworker.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/threadcontainer.h>

#include <QTime>

class QObject;

DPFILEOPERATIONS_BEGIN_NAMESPACE
class DoCopyFileWorker;
class FileOperateBaseWorker : public AbstractWorker, public QEnableSharedFromThis<FileInfo>
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
        FileInfoPointer fromInfo { nullptr };
        FileInfoPointer toInfo { nullptr };
    };

public:
    DFileInfoPointer doCheckFile(const DFileInfoPointer &fromInfo,
                                 const DFileInfoPointer &toInfo,
                                 const QString &fileName,
                                 bool *skip);
    DFileInfoPointer doCheckNewFile(const DFileInfoPointer &fromInfo,
                                    const DFileInfoPointer &toInfo,
                                    QString &fileNewName,
                                    bool *skip, bool isCountSize = false);
    bool checkFileSize(qint64 size, const QUrl &fromUrl,
                       const QUrl &toUrl, bool *skip);
    bool checkDiskSpaceAvailable(const QUrl &fromUrl, const QUrl &toUrl, bool *skip);
    bool checkTotalDiskSpaceAvailable(const QUrl &fromUrl, const QUrl &toUrl, bool *skip);
    void setTargetPermissions(const QUrl &fromUrl, const QUrl &toUrl);
    void setAllDirPermisson();
    void determineCountProcessType();
    qint64 getWriteDataSize();
    qint64 getTidWriteSize();
    qint64 getSectorsWritten();
    void readAheadSourceFile(const DFileInfoPointer &fileInfo);
    void syncFilesToDevice();
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const bool isTo = false,
                                                           const QString &errorMsg = QString(),
                                                           const bool errorMsgAll = false);
    // notify
    void emitSpeedUpdatedNotify(const qint64 &writSize);

    bool deleteFile(const QUrl &fromUrl, const QUrl &toUrl, bool *result, const bool force = false);
    bool deleteDir(const QUrl &fromUrl, const QUrl &toUrl, bool *result, const bool force = false);
    bool copyFileFromTrash(const QUrl &urlSource, const QUrl &urlTarget, dfmio::DFile::CopyFlag flag);

    bool copyAndDeleteFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &targetPathInfo, const DFileInfoPointer &toInfo,
                           bool *result);
    bool createSystemLink(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                          const bool followLink, const bool doCopy,
                          bool *result);
    bool canWriteFile(const QUrl &url) const;

    bool doCopyFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip);
    bool checkAndCopyFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip);
    bool checkAndCopyDir(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip);

protected:
    void waitThreadPoolOver();
    void initCopyWay();
    QUrl trashInfo(const DFileInfoPointer &fromInfo);
    QString fileOriginName(const QUrl &trashInfoUrl);
    void removeTrashInfo(const QUrl &trashInfoUrl);

private:
    void setSkipValue(bool *skip, AbstractJobHandler::SupportAction action);
    void initThreadCopy();
    void initSignalCopyWorker();
    bool actionOperating(const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip);
    QUrl createNewTargetUrl(const DFileInfoPointer &toInfo, const QString &fileName);
    bool doCopyLocalFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo);
    bool doCopyOtherFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip);
    bool doCopyLocalBigFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip);

private:   // do copy local big file
    bool doCopyLocalBigFileResize(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, int toFd, bool *skip);
    char *doCopyLocalBigFileMap(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, int fd, const int per, bool *skip);
    void memcpyLocalBigFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, char *fromPoint, char *toPoint);
    void doCopyLocalBigFileClear(const size_t size, const int fromFd,
                                 const int toFd, char *fromPoint, char *toPoint);
    int doOpenFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, const bool isTo,
                   const int openFlag, bool *skip);

protected Q_SLOTS:
    void emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                         const bool isTo = false, const quint64 id = 0, const QString &errorMsg = QString(),
                         const bool allUsErrorMsg = false) override;
    virtual void emitCurrentTaskNotify(const QUrl &from, const QUrl &to) override;
    void skipMemcpyBigFile(const QUrl url);

private:
    QVariant
    checkLinkAndSameUrl(const DFileInfoPointer &fromInfo,
                        const DFileInfoPointer &newTargetInfo,
                        const bool isCountSize);
    QVariant doActionReplace(const DFileInfoPointer &fromInfo,
                             const DFileInfoPointer &newTargetInfo,
                             const bool isCountSize);
    QVariant doActionMerge(const DFileInfoPointer &fromInfo,
                           const DFileInfoPointer &newTargetInfo, const bool isCountSize);

protected:
    DFileInfoPointer targetInfo { nullptr };   // target file infor pointer
    CountWriteSizeType countWriteType { CountWriteSizeType::kCustomizeType };   // get write size type
    long copyTid = { -1 };   // 使用 /pric/[pid]/task/[tid]/io 文件中的的 writeBytes 字段的值作为判断已写入数据的依据
    qint64 targetDeviceStartSectorsWritten { 0 };   // 记录任务开始时目标磁盘设备已写入扇区数
    QString targetSysDevPath;   // /sys/dev/block/x:x
    qint16 targetLogSecionSize { 512 };   // 目标设备逻辑扇区大小
    qint8 targetIsRemovable { 1 };   // 目标磁盘设备是不是可移除或者热插拔设备
    DirPermissonList dirPermissonList;   // dir set Permisson list
    QFuture<void> syncResult;
    QString blocakTargetRootPath;
    QList<QUrl> syncFiles;

    std::atomic_int threadCopyFileCount { 0 };
    QList<DFileInfoPointer> cutAndDeleteFiles;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATEBASEWORKER_H
