/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef ABSTRACTWORKER_H
#define ABSTRACTWORKER_H

#include "dfm_common_service_global.h"
#include "fileoperationsutils.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/utils/filestatisticsjob.h"

#include <QObject>
#include <QUrl>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>
#include <QSharedPointer>
#include <QTime>

namespace dfm_service_common {
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
     * \brief errorNotify 错误信息，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobtypeKey任务类型，类型JobType）、source（源文件url，类型：QUrl）
     * 、target（源文件url，类型：QUrl）、errorType（错误类型，类型：JobErrorType）、sourceMsg（源文件url拼接的显示字符串，
     * 类型：QString）、targetMsg（目标文件url拼接的显示字符串，类型：QString）、kErrorMsgKey（错误信息字符串，类型：QString）、
     * kActionsKey（支持的操作，类型：SupportActions）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有存在sourceMsg（显示任务的左第一个label的显示，类型：QString）、
     * targetMsg（显示任务的左第二个label的显示，类型：QString）、kErrorMsgKey（显示任务的左第三个label的显示，类型：QString）、
     * kActionsKey（支持的操作，用来显示那些按钮，类型：SupportActions）
     */
    void errorNotify(const JobInfoPointer jobInfo);
    /*!
     * \brief speedUpdatedNotify 速度更新信号，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobtypeKey（任务类型，类型JobType）、kSpeedKey（源文件url拼接的
     * 显示字符串，类型：QString）、kRemindTimeKey（目标文件url拼接的显示字符串，类型：QString）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有存在kSpeedKey（显示任务的右第一个label的显示，类型：QString）、
     * kRemindTimeKey（（显示任务的右第二个label的显示，类型：QString）
     */
    void speedUpdatedNotify(const JobInfoPointer jobInfo);

    void requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType type, const QList<QUrl> list);
signals:   // update proccess timer use
    void startUpdateProgressTimer();

public:
    virtual void doOperateWork(AbstractJobHandler::SupportActions actions);

protected:
    virtual void stop();
    virtual void pause();
    virtual void resume();
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
                                 const QString &errorMsg = QString());
    virtual AbstractJobHandler::SupportActions supportActions(const AbstractJobHandler::JobErrorType &error);
    bool isStopped();
    JobInfoPointer createCopyJobInfo(const QUrl &from, const QUrl &to);

protected slots:
    virtual bool doWork();
    virtual void onUpdateProgress() {}
    virtual void onStatisticsFilesSizeFinish();
    virtual void onStatisticsFilesSizeUpdate(qint64 size);

protected:
    void initHandleConnects(const JobHandlePointer handle);
    explicit AbstractWorker(QObject *parent = nullptr);
    QString formatFileName(const QString &fileName);
    void saveOperations();

public:
    virtual ~AbstractWorker();

public:
    QSharedPointer<DFMBASE_NAMESPACE::FileStatisticsJob> statisticsFilesSizeJob { nullptr };   // statistics file info async
    QSharedPointer<QThread> updateProgressThread { nullptr };   // update progress timer thread
    QSharedPointer<UpdateProgressTimer> updateProgressTimer { nullptr };   // update progress timer

    JobHandlePointer handle { nullptr };   // handle
    QSharedPointer<LocalFileHandler> handler { nullptr };   // file base operations handler

    AbstractJobHandler::JobType jobType { AbstractJobHandler::JobType::kUnknow };   // current task type
    AbstractJobHandler::JobFlags jobFlags { AbstractJobHandler::JobFlag::kNoHint };   // job flag
    AbstractJobHandler::SupportAction currentAction { AbstractJobHandler::SupportAction::kNoAction };   // current action
    std::atomic_bool rememberSelect { false };
    std::atomic_bool stopWork { false };
    AbstractJobHandler::JobState currentState = AbstractJobHandler::JobState::kUnknowState;   // current state

    QAtomicInteger<qint64> sourceFilesTotalSize { 0 };   // total size of all source files
    QAtomicInteger<qint64> sourceFilesCount { 0 };   // source files count
    quint16 dirSize { 0 };   // size of dir

    QList<QUrl> sourceUrls;   // source urls
    QUrl targetUrl;   // target dir url
    QList<QUrl> allFilesList;   // all files(contains children)
    QQueue<Qt::HANDLE> errorThreadIdQueue;   // Thread queue for processing errors
    QList<QUrl> completeSourceFiles;   // List of all copied files
    QList<QUrl> completeTargetFiles;   // List of all complete target files
    QVariantList completeCustomInfos;
    QList<AbstractFileInfoPointer> precompleteTargetFileInfo;   // list prepare complete target file info
    bool isSourceFileLocal { false };   // source file on local device
    bool isTargetFileLocal { false };   // target file on local device
    bool isConvert { false };   // is convert operation
    QTime timeElapsed;

    QWaitCondition handlingErrorCondition;
    QMutex handlingErrorQMutex;
    QWaitCondition waitCondition;
    QWaitCondition errorCondition;   //  Condition variables that block other bad threads
    QMutex errorThreadIdQueueMutex;   // Condition variables that block other bad threads mutex
    QMutex cacheCopyingMutex;
};

}

#endif   // ABSTRACTWORKER_H
