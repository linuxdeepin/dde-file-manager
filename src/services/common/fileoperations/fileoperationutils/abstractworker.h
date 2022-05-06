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

DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class UpdateProccessTimer;

class AbstractWorker : public QObject
{
    friend class AbstractJob;
    Q_OBJECT
    virtual void setWorkArgs(const JobHandlePointer &handle, const QList<QUrl> &sourceUrls, const QUrl &targetUrl = QUrl(),
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
    void proccessChangedNotify(const JobInfoPointer JobInfo);
    /*!
     * @brief stateChanged 任务状态发生改变，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobStateKey（当前任务执行的状态,类型：JobState）和存在kJobtypeKey（任务类型，类型JobType）
     * 在我们自己提供的dailog服务中，这个VarintMap必须存在kJobStateKey（当前任务执行的状态，类型：JobState）用来展示暂停和开始按钮状态
     */
    void stateChangedNotify(const JobInfoPointer JobInfo);
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
    void currentTaskNotify(const JobInfoPointer JobInfo);
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
    void errorNotify(const JobInfoPointer JobInfo);
    /*!
     * \brief speedUpdatedNotify 速度更新信号，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobtypeKey（任务类型，类型JobType）、kSpeedKey（源文件url拼接的
     * 显示字符串，类型：QString）、kRemindTimeKey（目标文件url拼接的显示字符串，类型：QString）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有存在kSpeedKey（显示任务的右第一个label的显示，类型：QString）、
     * kRemindTimeKey（（显示任务的右第二个label的显示，类型：QString）
     */
    void speedUpdatedNotify(const JobInfoPointer JobInfo);

    void requestShowRestoreFailedDialog(const QList<QUrl> &urls);
signals:   // update proccess timer use
    void startUpdateProccessTimer();

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
    virtual void emitProccessChangedNotify(const qint64 &writSize);
    virtual void emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                                 const QString &errorMsg = QString());
    virtual AbstractJobHandler::SupportActions supportActions(const AbstractJobHandler::JobErrorType &error)
    {
        Q_UNUSED(error);
        return AbstractJobHandler::SupportAction::kNoAction;
    }
    bool isStopped();
    JobInfoPointer createCopyJobInfo(const QUrl &from, const QUrl &to);

protected slots:
    virtual bool doWork();
    virtual void onUpdateProccess() {}
    virtual void onStatisticsFilesSizeFinish();

protected:
    void initHandleConnects(const JobHandlePointer &handle);
    explicit AbstractWorker(QObject *parent = nullptr);
    QString formatFileName(const QString &fileName);
    QString getNonExistFileName(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer targetDir);
    void saveOperations();

public:
    virtual ~AbstractWorker();

public:
    QSharedPointer<DFMBASE_NAMESPACE::FileStatisticsJob> statisticsFilesSizeJob { nullptr };   // statistics file info async
    QSharedPointer<QThread> updateProccessThread { nullptr };   // update proccess timer thread
    QSharedPointer<UpdateProccessTimer> updateProccessTimer { nullptr };   // update proccess timer

    JobHandlePointer handle { nullptr };   // handle
    QSharedPointer<LocalFileHandler> handler { nullptr };   // file base operations handler

    AbstractJobHandler::JobType jobType { AbstractJobHandler::JobType::kUnknow };   // current task type
    AbstractJobHandler::JobFlags jobFlags { AbstractJobHandler::JobFlag::kNoHint };   // job flag
    AbstractJobHandler::SupportAction currentAction { AbstractJobHandler::SupportAction::kNoAction };   // current action
    AbstractJobHandler::JobState currentState = AbstractJobHandler::JobState::kUnknowState;   // current state

    QAtomicInteger<qint64> sourceFilesTotalSize { 0 };   // total size of all source files
    QAtomicInteger<qint64> sourceFilesCount { 0 };   // source files count
    quint16 dirSize { 0 };   // size of dir

    QList<QUrl> sourceUrls;   // source urls
    QUrl targetUrl;   // target dir url
    QList<QUrl> allFilesList;   // all files(contains children)
    QQueue<Qt::HANDLE> errorThreadIdQueue;   // Thread queue for processing errors
    QList<QUrl> completeFiles;   // List of all copied files
    QList<QUrl> completeTargetFiles;   // List of all complete target files
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

DSC_END_NAMESPACE

#endif   // ABSTRACTWORKER_H
