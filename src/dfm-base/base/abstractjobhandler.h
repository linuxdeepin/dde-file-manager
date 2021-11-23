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
#ifndef DABSTRACTJOBHANDLER_H
#define DABSTRACTJOBHANDLER_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QSharedPointer>
DFMBASE_BEGIN_NAMESPACE

class AbstractJobHandler : public QObject
{
    Q_OBJECT
public:
    enum class JobOperate : uint8_t {
        kPauseOperate,   // 暂停操作
        kResumeOperate,   // 恢复操作
        kStopOperate,   // 停止操作
        kReplaceOperate,   // 替换操作
        kMergeDirOperate,   // 合并目录操作
        kSkipOperatre,   // 跳过操作
        kCancelOperate,   // 退出操作
        kUnknowOperate = 255
    };
    enum class JobState : uint8_t {
        kStartState,   // 开始状态
        kRunningState,   // 运行状态
        kPauseState,   // 暂停状态
        kStopState,   // 停止状态
        kUnknowState = 255
    };
    explicit AbstractJobHandler(QObject *parent = nullptr);
    virtual ~AbstractJobHandler();
    virtual qreal currentJobProcess() const;
    virtual qint64 totalSize() const;
    virtual qint64 currentSize() const;
    virtual JobState currentState() const;
    virtual bool operate(const JobOperate &op);
signals:
    /*!
     * @brief proccessChanged 当前任务的进度变化信号
     * 任务实时进度变化
     * param qint64 &current 当前任务执行的进度
     * param qint64 &total 当前任务文件的总大小
     * @return bool 操作是否成功
     */
    void proccessChanged(const qint64 &current, const qint64 &total);
    /*!
     * @brief proccessChanged 当前任务的进度发生变化信号
     * 任务状态发生改变
     * param JobState &state 当前任务执行的状态
     * @return bool 操作是否成功
     */
    void stateChanged(const JobState &state);
};
DFMBASE_END_NAMESPACE

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractJobHandler> JobHandlePointer;
Q_DECLARE_METATYPE(JobHandlePointer)

#endif   // DABSTRACTJOBHANDLER_H
