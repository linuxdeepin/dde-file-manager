/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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
