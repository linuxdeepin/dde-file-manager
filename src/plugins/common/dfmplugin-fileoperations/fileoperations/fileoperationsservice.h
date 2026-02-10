// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATIONSSERVICE_H
#define FILEOPERATIONSSERVICE_H

#include "dfmplugin_fileoperations_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <QObject>
#include <QMap>

DPFILEOPERATIONS_BEGIN_NAMESPACE
class FileOperationsService : public QObject
{
    Q_OBJECT

public:
    explicit FileOperationsService(QObject *parent = nullptr);
    virtual ~FileOperationsService() override;

    JobHandlePointer copy(const QList<QUrl> &sources, const QUrl &target,
                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer copyFromTrash(const QList<QUrl> &sources, const QUrl &target,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer moveToTrash(const QList<QUrl> &sources,
                                 const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer restoreFromTrash(const QList<QUrl> &sources, const QUrl &target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer deletes(const QList<QUrl> &sources,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags
                             &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer cut(const QList<QUrl> &sources, const QUrl &target,
                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer cleanTrash(const QList<QUrl> &sources);

private slots:
    void handleWorkerFinish();

private:
    QMap<QString, JobHandlePointer> jobs;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATIONSSERVICE_H
