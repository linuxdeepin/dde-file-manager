// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECOPYMOVEJOB_H
#define FILECOPYMOVEJOB_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationsservice.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/dialogmanager.h>

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
                                 const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags
                                 = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint,
                                 const bool isInit = true);
    JobHandlePointer restoreFromTrash(const QList<QUrl> &sources, const QUrl &target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint,
                                      const bool isInit = true);
    JobHandlePointer deletes(const QList<QUrl> &sources,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags
                             &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint,
                             const bool isInit = true);
    JobHandlePointer cut(const QList<QUrl> &sources, const QUrl &target,
                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags
                         = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint,
                         const bool isInit = true);
    JobHandlePointer cleanTrash(const QList<QUrl> &sources);
    void initArguments(const JobHandlePointer handler,
                       const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags =
            DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    void startAddTaskTimer(const JobHandlePointer handler, const bool isRemote);

private:
    bool getOperationsAndDialogService();

private slots:
    void onHandleAddTask();
    void onHandleAddTaskWithArgs(const JobInfoPointer info);
    void onHandleTaskFinished(const JobInfoPointer info);

private:
    QMap<JobHandlePointer, QSharedPointer<QTimer>> copyMoveTask;
    QSharedPointer<QMutex> copyMoveTaskMutex { nullptr };
    QSharedPointer<FileOperationsService> operationsService { nullptr };
    DFMBASE_NAMESPACE::DialogManager *dialogManager { nullptr };
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILECOPYMOVEJOB_H
