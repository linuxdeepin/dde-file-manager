// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTWORKER_H
#define ABSTRACTWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperationsutils.h"
#include "workerdata.h"
#include "docopyfileworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/filescanner.h>

#include <dfm-framework/event/event.h>

#include <QObject>
#include <QUrl>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>
#include <QSharedPointer>
#include <QTimer>
#include <QThreadPool>

DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class UpdateProgressTimer;
class AbstractWorker : public QObject
{
    friend class AbstractJob;
    Q_OBJECT
    virtual void setWorkArgs(const JobHandlePointer handle, const QList<QUrl> &sourceUrls, const QUrl &targetUrl = QUrl(),
                             const AbstractJobHandler::JobFlags &flags = AbstractJobHandler::JobFlag::kNoHint);

public:
    enum class CountWriteSizeType : quint8 {
        kTidType,   // Read thread IO write size 使用 /pric/[pid]/task/[tid]/io 文件中的的 writeBytes 字段的值作为判断已写入数据的依据
        kWriteBlockType,   // Read write block device write block size
        kCustomizeType
    };

    enum class SyncType {
        kBlocking,
        kNonblocking
    };

signals:
    /*!
     * @brief proccessChanged 当前任务的进度变化信号，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobtypeKey（任务类型，类型JobType）,kCurrentProccessKey（当前任务执行的进度，类型qint64），
     * kTotalSizeKey（当前任务文件的总大小，如果统计文件数量没有完成，值为-1,类型qint64）三个字段
     * 在我们自己提供的dailog服务中，这个VarintMap必须有kCurrentProccessKey（当前任务执行的进度，类型qint64）和
     * kTotalSizeKey（当前任务文件的总大小，如果统计文件数量没有完成，值为-1，类型qint64）值来做文件进度的展示
     */
    void progressChangedNotify(const JobInfoPointer jobInfo);
    /*!
     * @brief stateChanged 任务状态发生改变，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobStateKey（当前任务执行的状态,类型：JobState）和存在kJobtypeKey（任务类型，类型JobType）
     * 在我们自己提供的dailog服务中，这个VarintMap必须存在kJobStateKey（当前任务执行的状态，类型：JobState）用来展示暂停和开始按钮状态
     */
    void stateChangedNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief currentTaskNotify 当前任务的信息变化，此信号都可能是异步连接，所以所有参数都没有使用引用
     * 例如：拷贝文件时，正在拷贝a文件到b目录，发送此信号
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobtypeKey（任务类型，类型JobType）、kSourceUrlKey（源文件url，类型：QUrl）
     * 、kTargetUrlKey（源文件url，类型：QUrl）、kSourceMsgKey（源文件url拼接的显示字符串，类型：QString）和kTargetMsgKey（目标文件url拼接的显示字符串，
     * 类型：QString）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有存在kSourceMsgKey（显示任务的左第一个label的显示，类型：QString）
     * 和kTargetMsgKey显示任务的左第二个label的显示，类型：QString）
     */
    void currentTaskNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief finishedNotify 任务完成
     */
    void finishedNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief speedUpdatedNotify 速度更新信号，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobtypeKey（任务类型，类型JobType）、kSpeedKey（源文件url拼接的
     * 显示字符串，类型：QString）、kRemindTimeKey（目标文件url拼接的显示字符串，类型：QString）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有存在kSpeedKey（显示任务的右第一个label的显示，类型：QString）、
     * kRemindTimeKey（（显示任务的右第二个label的显示，类型：QString）
     */
    void speedUpdatedNotify(const JobInfoPointer jobInfo);

    void removeTaskWidget();

    void requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType type, const QList<QUrl> &list);
    void requestShowFailedDialog(const QMap<QUrl, QString> &failedInfo);
    void workerFinish();
    void requestSaveRedoOperation(const QString &token, const qint64 deleteFirstFileSize);
    void startWork();
    void errorNotify(const JobInfoPointer jobInfo);
    void retryErrSuccess(const quint64 id);
    void requestTaskDailog();

    void fileRenamed(const QUrl &old, const QUrl &cur);
    void fileDeleted(const QUrl &url);
    void fileAdded(const QUrl &url);

public:
    void doOperateWork(AbstractJobHandler::SupportActions actions, AbstractJobHandler::JobErrorType error = AbstractJobHandler::JobErrorType::kNoError, const quint64 id = 0);

protected:
    virtual void stop();
    virtual void startCountProccess();
    virtual bool statisticsFilesSize();
    virtual bool stateCheck();
    virtual bool workerWait();
    virtual void setStat(const AbstractJobHandler::JobState &stat);
    virtual bool initArgs();
    virtual void endWork();
    virtual void emitStateChangedNotify();
    virtual void emitCurrentTaskNotify(const QUrl &from, const QUrl &to);
    virtual void emitProgressChangedNotify(const qint64 &writSize);
    virtual void emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                                 const bool isTo = false, const quint64 id = 0, const QString &errorMsg = QString(),
                                 const bool allUsErrorMsg = false);

    // Sync before stop for external devices
    virtual bool needsSync() const { return false; }
    virtual void performSync() { }
    virtual void performAsyncSync() { }

protected slots:
    virtual bool doWork();
    virtual void onUpdateProgress() { }
    virtual void onStatisticsFilesSizeFinish(const DFMBASE_NAMESPACE::FileScanner::ScanResult &result);
    virtual void onStatisticsFilesSizeUpdate(const DFMBASE_NAMESPACE::FileScanner::ScanResult &result);

protected:
    void initHandleConnects(const JobHandlePointer handle);
    explicit AbstractWorker(QObject *parent = nullptr);
    QString formatFileName(const QString &fileName);
    void saveOperations();
    bool isStopped();
    JobInfoPointer createCopyJobInfo(const QUrl &from, const QUrl &to,
                                     const AbstractJobHandler::JobErrorType error = AbstractJobHandler::JobErrorType::kNoError);

    void resumeAllThread();
    void resumeThread(const QList<quint64> &errorIds);
    void pauseAllThread();
    void stopAllThread();
    void checkRetry();
    void pause();
    void resume();
    void getAction(AbstractJobHandler::SupportActions actions);
    QUrl parentUrl(const QUrl &url);
    void syncFilesToDevice();

    static dfmbase::FileInfo::FileType fileType(const DFileInfoPointer &info);

public:
    virtual ~AbstractWorker();

public:
    QSharedPointer<DFMBASE_NAMESPACE::FileScanner> statisticsFilesSizeJob { nullptr };   // statistics file info async
    QSharedPointer<UpdateProgressTimer> updateProgressTimer { nullptr };   // update progress timer

    JobHandlePointer handle { nullptr };   // handle
    QSharedPointer<LocalFileHandler> localFileHandler { nullptr };   // file base operations handler

    AbstractJobHandler::JobType jobType { AbstractJobHandler::JobType::kUnknow };   // current task type
    AbstractJobHandler::SupportAction currentAction { AbstractJobHandler::SupportAction::kNoAction };   // current action

    std::atomic_bool stopWork { false };
    AbstractJobHandler::JobState currentState = AbstractJobHandler::JobState::kUnknowState;   // current state

    QAtomicInteger<qint64> sourceFilesTotalSize { 0 };   // total size of all source files
    QAtomicInteger<qint64> sourceFilesCount { 0 };   // source files count

    QList<QUrl> sourceUrls;   // source urls
    QUrl targetUrl;   // target dir url
    QUrl targetOrgUrl;   // target origin url
    QList<QUrl> allFilesList;   // all files(contains children)
    QList<QUrl> completeSourceFiles;   // List of all copied files
    QList<QUrl> completeTargetFiles;   // List of all complete target files
    QVariantList completeCustomInfos;
    QList<DFileInfoPointer> precompleteTargetFileInfo;   // list prepare complete target file info
    bool isSourceFileLocal { false };   // source file on local device
    bool isTargetFileLocal { false };   // target file on local device
    bool supportSetPermission { true };   // source file on mtp
    bool supportDfmioCopy { true };   // source file on mtp
    bool isConvert { false };   // is convert operation
    QSharedPointer<WorkerData> workData { nullptr };
    QSharedPointer<DoCopyFileWorker> copyOtherFileWorker { nullptr };
    std::atomic_bool exblockThreadStarted { false };
    QElapsedTimer timeElapsed;

    QWaitCondition waitCondition;
    QMutex mutex;
    QVector<QSharedPointer<DoCopyFileWorker>> threadCopyWorker;
    int threadCount { 4 };
    std::atomic_bool retry { false };
    QSharedPointer<QThreadPool> threadPool { nullptr };
    static std::atomic_bool bigFileCopy;
    QAtomicInteger<qint64> bigFileSize { 0 };   // bigger than this is big file
    QElapsedTimer *speedtimer { nullptr };   // time eslape
    std::atomic_int64_t elapsed { 0 };
    std::atomic_int64_t deleteFirstFileSize { false };
    bool isCutMerge { false };
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // ABSTRACTWORKER_H
