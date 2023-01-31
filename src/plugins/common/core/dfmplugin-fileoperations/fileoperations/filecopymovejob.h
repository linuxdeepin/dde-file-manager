/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef FILECOPYMOVEJOB_H
#define FILECOPYMOVEJOB_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationsservice.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/dialogmanager.h"

#include <QObject>
#include <QPointer>

class QTimer;

DPFILEOPERATIONS_BEGIN_NAMESPACE

class FileCopyMoveJob : public QObject
{
    Q_OBJECT
public:
    explicit FileCopyMoveJob(QObject *parent = nullptr);
    ~FileCopyMoveJob() override;

    JobHandlePointer copy(const QList<QUrl> &sources, const QUrl &target,
                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer copyFromTrash(const QList<QUrl> &sources, const QUrl &target,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer moveToTrash(const QList<QUrl> &sources,
                                 const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer restoreFromTrash(const QList<QUrl> &sources, const QUrl &target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer deletes(const QList<QUrl> &sources,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer cut(const QList<QUrl> &sources, const QUrl &target,
                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer cleanTrash(const QList<QUrl> &sources);

private:
    bool getOperationsAndDialogService();
    void initArguments(const JobHandlePointer handler);
private slots:
    void onHandleAddTask();
    void onHandleAddTaskWithArgs(const JobInfoPointer info);
    void onHandleTaskFinished(const JobInfoPointer info);

private:
    QMap<JobHandlePointer, QSharedPointer<QTimer>> copyMoveTask;
    QSharedPointer<QMutex> copyMoveTaskMutex { nullptr };
    QSharedPointer<QMutex> getOperationsAndDialogServiceMutex { nullptr };
    QSharedPointer<FileOperationsService> operationsService { nullptr };
    DFMBASE_NAMESPACE::DialogManager *dialogManager { nullptr };
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILECOPYMOVEJOB_H
