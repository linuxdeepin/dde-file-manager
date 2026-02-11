// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEEVENTRECEIVER_H
#define TRASHFILEEVENTRECEIVER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationsservice.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-io/denumerator.h>

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

Q_SIGNALS:
    void cleanTrashUrls(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback);

public slots:
    void handleOperationMoveToTrash(const quint64 windowId,
                                    const QList<QUrl> sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    void handleOperationMoveToTrash(const quint64 windowId,
                                    const QList<QUrl> sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle,
                                    const QVariant custom,
                                    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    void handleOperationRestoreFromTrash(const quint64 windowId,
                                         const QList<QUrl> sources, const QUrl target,
                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    void handleOperationRestoreFromTrash(const quint64 windowId,
                                         const QList<QUrl> sources, const QUrl target,
                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle,
                                         const QVariant custom,
                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    void handleOperationCleanTrash(const quint64 windowId,
                                   const QList<QUrl> sources,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                   DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    void handleOperationCleanTrash(const quint64 windowId,
                                   const QList<QUrl> sources,
                                   DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle,
                                   const QVariant custom,
                                   DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    void handleOperationCopyFromTrash(const quint64 windowId,
                                      const QList<QUrl> &sources, const QUrl &target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    void handleOperationCopyFromTrash(const quint64 windowId,
                                      const QList<QUrl> &sources, const QUrl &target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle,
                                      const QVariant custom,
                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);
    void handleSaveRedoOpt(const QString &token, const bool moreThanZero);
    void handleOperationUndoMoveToTrash(const quint64 windowId,
                                        const QList<QUrl> &sources,
                                        const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                        const QVariantMap &op);
    void handleOperationUndoRestoreFromTrash(const quint64 windowId,
                                            const QList<QUrl> &sources, const QUrl &target,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                            DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                            const QVariantMap &op);
private slots:
    JobHandlePointer onCleanTrashUrls(const quint64 windowId, const QList<QUrl> &sources,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback);

private:
    explicit TrashFileEventReceiver(QObject *parent = nullptr);

    JobHandlePointer doMoveToTrash(const quint64 windowId, const QList<QUrl> &sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                   DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                   const bool isInit = true);
    JobHandlePointer doRestoreFromTrash(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                        const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                        const bool isInit = true);
    JobHandlePointer doCopyFromTrash(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback);
    JobHandlePointer doCleanTrash(const quint64 windowId, const QList<QUrl> &sources, const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                  DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                  const bool showDelet = true);
    void countTrashFile(const quint64 windowId, const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback);

private:
    QSharedPointer<FileCopyMoveJob> copyMoveJob { nullptr };
    QSharedPointer<DFMIO::DEnumerator> trashIterator { nullptr };
    QFuture<void> future;
    QMap<QString, QVariantMap> undoOpts;
    QMutex undoLock;
    std::atomic_bool stoped { false };
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // TRASHFILEEVENTRECEIVER_H
