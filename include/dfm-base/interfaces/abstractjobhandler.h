// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DABSTRACTJOBHANDLER_H
#define DABSTRACTJOBHANDLER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QSharedPointer>
#include <QList>
#include <QUrl>
#include <QMutex>

class QWidget;

typedef QSharedPointer<QMap<quint8, QVariant>> JobInfoPointer;
// Q_DECLARE_METATYPE(JobInfoPointer);

namespace dfmbase {
class AbstractJobHandler : public QObject
{
    Q_OBJECT
public:
    enum class JobFlag {
        kNoHint = 0x00,
        kCopyFollowSymlink = 0x01,   // 是否拷贝链接文件的源文件
        kCopyAttributes = 0x02,   // 复制文件时携带它的扩展属性
        kCopyAttributesOnly = 0x04,   // 只复制文件的扩展属性
        kCopyToSelf = 0x08,   // 复制到自己
        kCopyRemoveDestination = 0x10,   // 复制文件前先删除已存在的
        kCopyResizeDestinationFile = 0x20,   // 复制文件前对目标文件执行resize操作
        kCopyIntegrityChecking = 0x40,   // 复制文件时进行完整性校验
        kDeleteForceDeleteFile = 0x80,   // 强制删除文件夹(去除文件夹的只读权限)
        kDontFormatFileName = 0x100,   // 拷贝时不处理文件名称
        kRevocation = 0x200,   // 拷贝时不处理文件名称
        kCopyRemote = 0x400,   // 深信服远程拷贝
        kRedo = 0x800,   // 重新执行（ctrl + Y）
        kCountProgressCustomize = 0x1000,   // 强制使用自己统计进度
    };
    Q_ENUM(JobFlag)
    Q_DECLARE_FLAGS(JobFlags, JobFlag)
    enum class JobState : uint8_t {
        kStartState = 0,   // 开始状态
        kRunningState = 1,   // 运行状态
        kPauseState = 2,   // 暂停状态
        kStopState = 3,   // 停止状态
        kUnknowState = 255
    };
    Q_ENUM(JobState)
    enum class StatisticState : uint8_t {
        kNoState = 0,   // 无状态
        kRunningState = 1,   // 运行状态
        kStopState = 2,   // 停止状态
    };
    Q_ENUM(StatisticState)
    enum class JobErrorType : uint8_t {
        kNoError = 0,   // 没有错误
        kNoSourceError = 1,   // 没有源文件
        kCancelError = 2,   // 退出错误
        kPermissionError = 3,   // 权限错误
        kSpecialFileError = 4,   // 特殊文件错误
        kFileExistsError = 5,   // 文件已存在
        kDirectoryExistsError = 6,   // 目录已存在
        kOpenError = 7,   // 打开文件错误
        kReadError = 8,   // 读取文件错误
        kWriteError = 9,   // 写入文件错误
        kSymlinkError = 10,   // 创建链接文件错误
        kMkdirError = 11,   // 创建目录错误
        kResizeError = 12,   // 调整文件大小错误
        kRemoveError = 13,   // 移除错误
        kRenameError = 14,   // 重命名错误
        kSymlinkToGvfsError = 15,   // 连接gvfs文件错误
        kUnknowUrlError = 16,   // 未知的url
        kNonexistenceError = 17,   // 不存在错误
        kIntegrityCheckingError = 18,   // 检查文件类型错误
        kFileSizeTooBigError = 19,   // 文件太大
        kNotEnoughSpaceError = 20,   // 没有剩余空间进行拷贝
        kTargetReadOnlyError = 21,   // 目标目录是只读
        kTargetIsSelfError = 22,   // 目标是自己
        kNotSupportedError = 23,   // 不支持的操作
        kPermissionDeniedError = 24,   // 权限错误
        kSeekError = 25,   // 文件移动错误
        kProrogramError = 26,   // 程序错误
        kDfmIoError = 27,   // 程序错误
        kDeleteFileError = 28,   // 删除文件错误
        kMakeStandardTrashError = 29,   // 回收站目录错误
        kGetRestorePathError = 30,   // 获取回收站文件信息失败
        kDeleteTrashFileError = 31,   // 删除回收站文件错误
        kIsNotTrashFileError = 32,   // 不是回收站文件
        kCreateParentDirError = 33,   // 创建父目录失败
        kFailedParseUrlOfTrash = 34,   // 解析trash的url失败 Failed to parse the url of trash
        kFailedObtainTrashOriginalFile = 35,   // 获取trash中原文件失败 Failed to obtain the trash original file
        kFileMoveToTrashError = 36,   // 文件移动到回收站错误
        kRetryReadOrWriteFailed = 37,   // 重试失败
        kCanNotAccessFile = 38,   // 无法访问远程文件
        kFileMoveToTrashNoSpace = 39,   // 文件移动到回收站存储器空间不足
        kUnknowError = 255,   // 未知错误
    };
    Q_ENUM(JobErrorType)
    enum class JobType : uint8_t {
        kCopyType = 0,   // 拷贝任务
        kCutType = 1,   // 剪切任务
        kDeleteType = 2,   // 删除任务
        kMoveToTrashType = 3,   // 移动到回收站
        kRestoreType = 4,   // 从回收站还原
        kCleanTrashType = 5,   // 清理回收站
        kUnknow = 255,
    };
    Q_ENUM(JobType)
    enum class SupportAction {
        kNoAction = 0x00,   // 没有操作
        kRetryAction = 0x01,   // 重试
        kReplaceAction = 0x02,   // 替换
        kMergeAction = 0x04,   // 合并
        kSkipAction = 0x08,   // 跳过
        kCoexistAction = 0x10,   // 共存
        kCancelAction = 0x20,   // 取消
        kEnforceAction = 0x40,   // 强制执行
        kRememberAction = 0x80,   // 记住当前动作
        kPauseAction = 0x100,   // 暂停操作
        kStopAction = 0x200,   // 停止操作
        kStartAction = 0x400,   // 开始操作
        kResumAction = 0x800,   // 恢复
        kPermanentlyDelete = 0x1000,   // 强制删除
    };
    Q_ENUM(SupportAction)
    Q_DECLARE_FLAGS(SupportActions, SupportAction)
    enum NotifyInfoKey : uint8_t {
        kJobtypeKey = 0,
        kCurrentProgressKey = 1,
        kTotalSizeKey = 2,
        kStatisticStateKey = 3,
        kJobStateKey = 4,
        kJobStateHideKey = 5,
        kSourceUrlKey = 6,
        kTargetUrlKey = 7,
        kErrorTypeKey = 8,
        kSourceMsgKey = 9,
        kTargetMsgKey = 10,
        kErrorMsgKey = 11,
        kActionsKey = 12,
        kSpeedKey = 13,
        kRemindTimeKey = 14,
        kCompleteFilesKey = 15,
        kCompleteTargetFilesKey = 16,
        kCompleteCustomInfosKey = 17,
        kJobHandlePointer = 18,
        kWorkerPointer = 19,
    };
    Q_ENUM(NotifyInfoKey)
    enum class NotifyType : uint8_t {
        kNotifyProccessChangedKey = 0,
        kNotifyStateChangedKey = 1,
        kNotifyCurrentTaskKey = 2,
        kNotifyFinishedKey = 3,
        kNotifySpeedUpdatedTaskKey = 4,
        kNotifyErrorTaskKey = 5,
    };
    Q_ENUM(NotifyType)
    enum FileNameAddFlag : uint8_t {
        kPrefix = 0,
        kSuffix = 1,
    };
    Q_ENUM(FileNameAddFlag)

    enum class DeleteDialogNoticeType : uint8_t {
        kEmptyTrash,
        kDeleteTashFiles
    };
    Q_ENUM(DeleteDialogNoticeType)

    enum class ShowDialogType : uint8_t {
        kRestoreFailed,
        kCopyMoveToSelf,
    };

    enum class CallbackKey : uint8_t {
        kWindowId,   // quint64 windowId
        kSuccessed,   // bool
        kSourceUrls,   // QList<QUrl>
        kTargets,   // QList<QUrl>
        kJobHandle,   // JobHandlePointer
        kCustom,   // QVariant
    };

    explicit AbstractJobHandler(QObject *parent = nullptr);
    virtual ~AbstractJobHandler();
    virtual qreal currentJobProcess() const;
    virtual qint64 totalSize() const;
    virtual qint64 currentSize() const;
    virtual JobState currentState() const;
    virtual void setSignalConnectFinished();
    virtual QMap<NotifyType, JobInfoPointer> getAllTaskInfo();
    virtual JobInfoPointer getTaskInfoByNotifyType(const NotifyType &notifyType);
Q_SIGNALS:   // 发送给任务调用者使用的信号
    /*!
     * @brief proccessChanged 当前任务的进度变化信号，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会有 kJobtypeKey （任务类型，类型JobType）,kCurrentProccessKey（当前任务执行的进度，类型qint64），
     * kTotalSizeKey（当前任务文件的总大小，如果统计文件数量没有完成，值为-1,类型qint64）三个字段
     * 在我们自己提供的dailog服务中，这个VarintMap必须有 kCurrentProccessKey （当前任务执行的进度，类型qint64）和
     * kTotalSizeKey（当前任务文件的总大小，如果统计文件数量没有完成，值为-1，类型qint64）值来做文件进度的展示
     */
    void proccessChangedNotify(const JobInfoPointer jobInfo);
    /*!
     * @brief stateChanged 任务状态发生改变，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会有kJobStateKey（当前任务执行的状态,类型：JobState）和存在kJobtypeKey（任务类型，类型JobType）
     * 在我们自己提供的dailog服务中，这个VarintMap必须存在 kJobStateKey （当前任务执行的状态，类型：JobState）用来展示暂停和开始按钮状态
     * kJobStateHideKey 隐藏暂停、停止按钮
     */
    void stateChangedNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief currentTaskNotify 当前任务的信息变化，此信号都可能是异步连接，所以所有参数都没有使用引用
     * 例如：拷贝文件时，正在拷贝a文件到b目录，发送此信号
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会有kJobtypeKey（任务类型，类型JobType）、kSourceUrlKey（源文件url，类型：QUrl）
     * 、kTargetUrlKey（源文件url，类型：QUrl）、kSourceMsgKey（源文件url拼接的显示字符串，类型：QString）和kTargetMsgKey（目标文件url拼接的显示字符串，
     * 类型：QString）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有有kSourceMsgKey（显示任务的左第一个label的显示，类型：QString）
     * 和kTargetMsgKey显示任务的左第二个label的显示，类型：QString）
     */
    void currentTaskNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief finishedNotify 任务完成
     * \param jobInfo 必须填写 NotifyInfoKey::kJobHandlePointer
     */
    void finishedNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief errorNotify 错误信息，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会有kJobtypeKey任务类型，类型JobType）、source（源文件url，类型：QUrl）
     * 、target（源文件url，类型：QUrl）、errorType（错误类型，类型：JobErrorType）、sourceMsg（源文件url拼接的显示字符串，
     * 类型：QString）、targetMsg（目标文件url拼接的显示字符串，类型：QString）、kErrorMsgKey（错误信息字符串，类型：QString）、
     * kActionsKey（支持的操作，类型：SupportActions）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有有sourceMsg（显示任务的左第一个label的显示，类型：QString）、
     * targetMsg（显示任务的左第二个label的显示，类型：QString）、kErrorMsgKey（显示任务的左第三个label的显示，类型：QString）、
     * kActionsKey（支持的操作，用来显示那些按钮，类型：SupportActions）
     */
    void errorNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief speedUpdatedNotify 速度更新信号，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会有kJobtypeKey（任务类型，类型JobType）、kSpeedKey（源文件url拼接的
     * 显示字符串，类型：QString）、kRemindTimeKey（目标文件url拼接的显示字符串，类型：QString）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有有kSpeedKey（显示任务的右第一个label的显示，类型：QString）、
     * kRemindTimeKey（（显示任务的右第二个label的显示，类型：QString）
     */
    void speedUpdatedNotify(const JobInfoPointer jobInfo);
    void requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType type, const QList<QUrl> list);
    void requestShowFailedDialog(const QMap<QUrl, QString> &failedInfo);
    void workerFinish();
    void requestRemoveTaskWidget();
    void requestSaveRedoOperation(const QString &token, const qint64 deleteFirstFileSize);
    void requestTaskDailog();
Q_SIGNALS:   // 发送给任务使用的信号
    /*!
     * \brief userAction 用户当前动作
     * \param action 当前的动作类型 这里的动作只能是action的一种和kRememberAction并存
     */
    void userAction(SupportActions actions);
public Q_SLOTS:
    void operateTaskJob(SupportActions actions);
    void onProccessChanged(const JobInfoPointer jobInfo);
    void onStateChanged(const JobInfoPointer jobInfo);
    void onCurrentTask(const JobInfoPointer jobInfo);
    void onError(const JobInfoPointer jobInfo);
    void onFinished(const JobInfoPointer jobInfo);
    void onSpeedUpdated(const JobInfoPointer jobInfo);

public:
    void start();

public:
    using CallbackArgus = QSharedPointer<QMap<AbstractJobHandler::CallbackKey, QVariant>>;
    using OperatorCallback = std::function<void(const CallbackArgus args)>;
    using OperatorHandleCallback = std::function<void(QSharedPointer<AbstractJobHandler>)>;

private:
    QAtomicInteger<bool> isSignalConnectOver { false };
    QMutex taskInfoMutex;
    QMap<NotifyType, JobInfoPointer> taskInfo;
};

}   // namespace dfmbase

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AbstractJobHandler::SupportActions)
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag)
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags)
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType)
// Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback);
// Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback);
using JobHandlePointer = QSharedPointer<DFMBASE_NAMESPACE::AbstractJobHandler>;
Q_DECLARE_METATYPE(JobHandlePointer)

#endif   // DABSTRACTJOBHANDLER_H
