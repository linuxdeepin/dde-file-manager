/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#include "dfm_common_service_global.h"

#include <QObject>

DSC_BEGIN_NAMESPACE

class AbstractJobHandler : public QObject
{
    Q_OBJECT
public:
    enum JobOperate {
        PauseOperate,                       //暂停操作
        ResumeOperate,                      //恢复操作
        StopOperate,                        //停止操作
        ReplaceOperate,                     //替换操作
        MergeDirOperate,                    //合并目录操作
        SkipOperatre,                       //跳过操作
        CancelOperate,                      //退出操作
        UnknowOperate = 255
    };
    enum JobState {
        StartState,
        RunningState,
        PauseState,
        StopState,
        UnknowState = 255
    };
    explicit AbstractJobHandler(QObject *parent = nullptr);
    virtual ~AbstractJobHandler();
    /**
     * @brief getCurrentJobProcess 获取当前任务的进度
     * 进度是当前任务拷贝的百分比*100,例如，拷贝过程中，当前拷贝了2300kb，总大小是250000kb，当前的进度是 (2300kb/250000kb) = 0.92
     * @return qreal 当前任务拷贝的进度
     */
    virtual qreal getCurrentJobProcess();
    /**
     * @brief getTotalSize 获取当前任务所有文件的总大小
     * 获取任务总共大小，如拷贝任务，获取就是当前任务拷贝的总大小，（拷贝任务可能才开始获取的不准确，开异步线程统计需要时间）
     * @return qreal 当前任务执行的进度
     */
    virtual qint64 getTotalSize();
    /**
     * @brief getCurrentSize 获取当前任务的进度（执行了任务的大小）
     * 获取任务总共大小，如拷贝任务，获取就是当前任务拷贝了的文件大小，（包含跳过了文件的大小）
     * @return qint64 当前任务的总大小
     */
    virtual qint64 getCurrentSize();
    /**
     * @brief getCurrentState 获取当前任务状态
     *
     * @return qreal 返回当前任务的状态
     */
    virtual qint64 getCurrentState();
    /**
     * @brief operate 对当前任务的操作
     * 进度是当前任务拷贝的百分比*100,例如，拷贝过程中，当前拷贝了2300kb，总大小是250000kb，当前的进度是 (2300kb/250000kb) = 0.92
     * param JOBOPERATE 对当前任务的操作
     * @return bool 操作是否成功
     */
    virtual bool operate(const JobOperate &op);
public Q_SIGNAL:
    /**
     * @brief proccessChanged 当前任务的进度变化信号
     * 任务实时进度变化
     * param qint64 &current 当前任务执行的进度
     * param qint64 &total 当前任务文件的总大小
     * @return bool 操作是否成功
     */
    void proccessChanged(const qint64 &current, const qint64 &total);
    /**
     * @brief proccessChanged 当前任务的进度发生变化信号
     * 任务状态发生改变
     * param JobState &state 当前任务执行的状态
     * @return bool 操作是否成功
     */
    void stateChanged(const JobState &state);

};

DSC_END_NAMESPACE

#endif // DABSTRACTJOBHANDLER_H
