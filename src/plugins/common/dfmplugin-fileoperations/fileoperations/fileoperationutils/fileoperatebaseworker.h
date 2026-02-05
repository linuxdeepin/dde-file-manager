// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATEBASEWORKER_H
#define FILEOPERATEBASEWORKER_H

#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/fileoperationutils/filecleanupmanager.h"
#include "fileoperations/fileoperationutils/filereplacer.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/threadcontainer.h>

#include <QTime>

class QObject;

DPFILEOPERATIONS_BEGIN_NAMESPACE
class DoCopyFileWorker;
class FileOperateBaseWorker : public AbstractWorker, public QEnableSharedFromThis<FileInfo>
{

public:
    // 替换目标信息：包含替换上下文和实际操作的目标路径
    struct ReplacementTarget
    {
        FileReplacementContext ctx;   // 替换上下文，如果需要临时文件则有效
        DFileInfoPointer actualInfo;   // 实际操作目标（可能是临时文件或原始目标）

        bool isUsingTemporary() const
        {
            return ctx.isReplacement();
        }
    };

    struct DirSetPermissonInfo
    {
        QFileDevice::Permissions permission;
        QUrl target;
    };

    using DirPermsissonPointer = QSharedPointer<DirSetPermissonInfo>;
    using DirPermissonList = DThreadList<DirPermsissonPointer>;

public:
    explicit FileOperateBaseWorker(QObject *parent = nullptr);
    virtual ~FileOperateBaseWorker() override;

    // Sync before stop overrides
    bool needsSync() const override;
    void performSync() override;
    void performAsyncSync() override;

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
    void setAllDirPermisson();
    void determineCountProcessType();
    qint64 getWriteDataSize();
    qint64 getTidWriteSize();
    qint64 getSectorsWritten();
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const bool isTo = false,
                                                           const QString &errorMsg = QString(),
                                                           const bool errorMsgAll = false);
    // notify
    void emitSpeedUpdatedNotify(const qint64 &writSize);

    bool copyFileFromTrash(const QUrl &urlSource, const QUrl &urlTarget, dfmio::DFile::CopyFlag flag);

    bool copyAndDeleteFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &targetPathInfo, const DFileInfoPointer &toInfo,
                           bool *result);
    bool createSystemLink(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                          const bool followLink, const bool doCopy,
                          bool *result);

    bool doCopyFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip);
    bool checkAndCopyFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip);
    bool checkAndCopyDir(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip);

protected:
    void waitThreadPoolOver();
    void initCopyWay();
    bool shouldUseBlockWriteType() const;
    QUrl trashInfo(const DFileInfoPointer &fromInfo);
    QString fileOriginName(const QUrl &trashInfoUrl);
    void removeTrashInfo(const QUrl &trashInfoUrl);
    void setSkipValue(bool *skip, AbstractJobHandler::SupportAction action);

    // 判断是否应该使用多线程本地复制（统一的判断接口）
    bool shouldUseMultiThreadCopy(const DFileInfoPointer &fromInfo) const;

protected Q_SLOTS:
    void emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                         const bool isTo = false, const quint64 id = 0, const QString &errorMsg = QString(),
                         const bool allUsErrorMsg = false) override;
    virtual void emitCurrentTaskNotify(const QUrl &from, const QUrl &to) override;

private:
    void initThreadCopy();
    void initSignalCopyWorker();
    QUrl createNewTargetUrl(const DFileInfoPointer &toInfo, const QString &fileName);
    bool doCopyLocalFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo);
    bool doCopyOtherFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip);
    bool doCopyLocalByRange(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip);
    void setExpectedSizeForTarget(const QUrl &targetUrl, qint64 size);

    // 延迟替换机制：批量应用所有待处理的替换
    bool applyAllPendingReplacements();

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
    qint16 targetLogicSectorSize { 512 };   // 目标设备逻辑扇区大小
    qint8 targetIsRemovable { 1 };   // 目标磁盘设备是不是可移除或者热插拔设备
    DirPermissonList dirPermissonList;   // dir set Permisson list
    FileCleanupManager cleanupManager;   // 管理不完整文件的清理

    std::atomic_int threadCopyFileCount { 0 };
    QList<DFileInfoPointer> cutAndDeleteFiles;

    // 延迟替换：待处理的替换上下文队列（主线程访问，无需锁）
    QList<ReplacementTarget> pendingReplacements;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATEBASEWORKER_H
