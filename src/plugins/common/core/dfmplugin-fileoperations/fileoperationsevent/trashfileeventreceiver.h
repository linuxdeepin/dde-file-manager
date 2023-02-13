// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEEVENTRECEIVER_H
#define TRASHFILEEVENTRECEIVER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationsservice.h"

#include "dfm-base/dbusservice/dbus_interface/operationsstackmanagerdbus_interface.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/dialogmanager.h"

#include <QObject>
#include <QPointer>
#include <QFileDevice>

DPFILEOPERATIONS_BEGIN_NAMESPACE
class FileCopyMoveJob;
class TrashFileEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashFileEventReceiver)

public:
    static TrashFileEventReceiver *instance();

public slots:
    void handleOperationMoveToTrash(const quint64 windowId,
                                    const QList<QUrl> sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                    DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationMoveToTrash(const quint64 windowId,
                                    const QList<QUrl> sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                    DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                    const QVariant custom,
                                    DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    void handleOperationRestoreFromTrash(const quint64 windowId,
                                         const QList<QUrl> sources, const QUrl target,
                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationRestoreFromTrash(const quint64 windowId,
                                         const QList<QUrl> sources, const QUrl target,
                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                         const QVariant custom,
                                         DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    void handleOperationCleanTrash(const quint64 windowId,
                                   const QList<QUrl> sources,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                   DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationCleanTrash(const quint64 windowId,
                                   const QList<QUrl> sources,
                                   DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                   const QVariant custom,
                                   DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    void handleOperationCopyFromTrash(const quint64 windowId,
                                      const QList<QUrl> sources, const QUrl target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                      DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationCopyFromTrash(const quint64 windowId,
                                      const QList<QUrl> sources, const QUrl target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                      DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                      const QVariant custom,
                                      DFMBASE_NAMESPACE::Global::OperatorCallback callback);

private:
    explicit TrashFileEventReceiver(QObject *parent = nullptr);

    JobHandlePointer doMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                   DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    JobHandlePointer doRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                        const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags, DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    JobHandlePointer doCopyFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags, DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    JobHandlePointer doCleanTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                  DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);

private:
    QSharedPointer<FileCopyMoveJob> copyMoveJob { nullptr };
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // TRASHFILEEVENTRECEIVER_H
