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
#ifndef ABSTRACTWORKER_H
#define ABSTRACTWORKER_H

#include "dfm_common_service_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "fileoperationsutils.h"

#include <QObject>
#include <QUrl>
#include <QSharedPointer>

Q_DECLARE_METATYPE(QSharedPointer<QList<QUrl>>);
class QWaitCondition;
class QMutex;
DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class StatisticsFilesSize;
class UpdateProccessTimer;
class AbstractWorker : public QObject
{
    friend class AbstractJob;
    Q_OBJECT
    virtual void setWorkArgs(const JobHandlePointer &handle, const QList<QUrl> &sources, const QUrl &target = QUrl(),
                             const AbstractJobHandler::JobFlags &flags = AbstractJobHandler::JobFlag::kNoHint);
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
    virtual void onStatisticsFilesSizeFinish(const SizeInfoPoiter sizeInfo);

protected:
    void initHandleConnects(const JobHandlePointer &handle);
    explicit AbstractWorker(QObject *parent = nullptr);

public:
    virtual ~AbstractWorker();

public:
    AbstractJobHandler::JobType jobType { AbstractJobHandler::JobType::kUnknow };   // current task type
    QSharedPointer<StatisticsFilesSize> statisticsFilesSizeJob { nullptr };   // 异步文件大小统计
    QAtomicInteger<qint64> sourceFilesTotalSize { 0 };   // 源文件的总大小
    QAtomicInteger<qint64> sourceFilesCount { 0 };   // source files count
    quint16 dirSize { 0 };   // 目录大小
    QList<QUrl> sources;   // 源文件
    QUrl target;   // 目标目录
    AbstractJobHandler::JobFlags jobFlags { AbstractJobHandler::JobFlag::kNoHint };   // 任务标志
    AbstractJobHandler::SupportAction currentAction { AbstractJobHandler::SupportAction::kNoAction };   // 当前的操作
    QSharedPointer<QWaitCondition> handlingErrorCondition { nullptr };
    AbstractJobHandler::JobState currentState = AbstractJobHandler::JobState::kUnknowState;   // current state
    bool isSourceFileLocal { false };   // 源文件是否在可以出设备上
    bool isTargetFileLocal { false };   // 目标文件是否在可以出设备上
    QSharedPointer<QWaitCondition> waitCondition { nullptr };   // 线程等待
    QSharedPointer<QMutex> conditionMutex { nullptr };   // 线程等待锁
    QSharedPointer<QList<QUrl>> allFilesList { nullptr };   // 所有源文件的统计文件
    QSharedPointer<LocalFileHandler> handler { nullptr };   // file base operations handler
    QSharedPointer<QQueue<Qt::HANDLE>> errorThreadIdQueue { nullptr };   // Thread queue for processing errors
    QSharedPointer<QWaitCondition> errorCondition { nullptr };   //  Condition variables that block other bad threads
    QSharedPointer<QMutex> errorThreadIdQueueMutex { nullptr };   // Condition variables that block other bad threads mutex
    QSharedPointer<UpdateProccessTimer> updateProccessTimer { nullptr };   // update proccess timer
    QSharedPointer<QThread> updateProccessThread { nullptr };   // update proccess timer thread
    QSharedPointer<QList<QUrl>> completeFiles { nullptr };   // List of all copied files
};

DSC_END_NAMESPACE

#endif   // ABSTRACTWORKER_H
