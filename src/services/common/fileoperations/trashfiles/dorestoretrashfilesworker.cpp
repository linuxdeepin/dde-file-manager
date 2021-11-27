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
#include "dorestoretrashfilesworker.h"

#include <QUrl>
#include <QDebug>

DSC_USE_NAMESPACE
DoRestoreTrashFilesWorker::DoRestoreTrashFilesWorker(QObject *parent)
    : AbstractWorker(parent)
{
}

DoRestoreTrashFilesWorker::~DoRestoreTrashFilesWorker()
{
}
/*!
 * \brief doOperateWork 处理用户的操作
 * \param actions 当前操作
 */
void DoRestoreTrashFilesWorker::doOperateWork(AbstractJobHandler::SupportActions actions)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStopAction)) {
        stop();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction)) {
        pause();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kRememberAction)) {
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kEnforceAction)) {
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kCancelAction)) {
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kCoexistAction)) {
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kSkipAction)) {
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kMergeAction)) {
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kReplaceAction)) {
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kRetryAction)) {
    }
}

void DoRestoreTrashFilesWorker::doWork()
{
    // ToDo::执行从回收站还原的业务逻辑
}

void DoRestoreTrashFilesWorker::stop()
{
    // ToDo::停止移动到回收站的业务逻辑
}

void DoRestoreTrashFilesWorker::pause()
{
    // ToDo::暂停移动到回收站的业务逻辑
}
