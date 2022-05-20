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
#ifndef FILEOPERATIONSEVENTRECEIVER_H
#define FILEOPERATIONSEVENTRECEIVER_H

#include "dfmplugin_fileoperations_global.h"
#include "services/common/fileoperations/fileoperationsservice.h"
#include "services/common/fileoperations/fileoperations_defines.h"

#include "dfm-base/dbusservice/dbus_interface/operationsstackmanagerdbus_interface.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/dialogmanager.h"

#include <QObject>
#include <QPointer>
#include <QFileDevice>

Q_DECLARE_METATYPE(QFileDevice::Permissions)
Q_DECLARE_METATYPE(QList<QUrl>)

DPFILEOPERATIONS_BEGIN_NAMESPACE
class FileCopyMoveJob;
class FileOperationsEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperationsEventReceiver)

public:
    static FileOperationsEventReceiver *instance();
    void connectService();

public slots:
    void handleOperationCopy(const quint64 windowId,
                             const QList<QUrl> sources,
                             const QUrl target,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationCut(const quint64 windowId,
                            const QList<QUrl> sources,
                            const QUrl target,
                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationMoveToTrash(const quint64 windowId,
                                    const QList<QUrl> sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                    DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationRestoreFromTrash(const quint64 windowId,
                                         const QList<QUrl> sources,
                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationDeletes(const quint64 windowId,
                                const QList<QUrl> sources,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationCleanTrash(const quint64 windowId,
                                   const QList<QUrl> sources,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                   DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    void handleOperationCopy(const quint64 windowId,
                             const QList<QUrl> sources,
                             const QUrl target,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                             const QVariant custom,
                             DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    void handleOperationCut(const quint64 windowId,
                            const QList<QUrl> sources,
                            const QUrl target,
                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                            const QVariant custom,
                            DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    void handleOperationMoveToTrash(const quint64 windowId,
                                    const QList<QUrl> sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                    DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                    const QVariant custom,
                                    DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    void handleOperationRestoreFromTrash(const quint64 windowId,
                                         const QList<QUrl> sources,
                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                         const QVariant custom,
                                         DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    void handleOperationDeletes(const quint64 windowId,
                                const QList<QUrl> sources,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                const QVariant custom,
                                DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    void handleOperationCleanTrash(const quint64 windowId,
                                   const QList<QUrl> sources,
                                   DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle,
                                   const QVariant custom,
                                   DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    bool handleOperationOpenFiles(const quint64 windowId,
                                  const QList<QUrl> urls);
    void handleOperationOpenFiles(const quint64 windowId,
                                  const QList<QUrl> urls,
                                  const QVariant custom,
                                  DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    bool handleOperationOpenFilesByApp(const quint64 windowId,
                                       const QList<QUrl> urls,
                                       const QList<QString> apps);
    void handleOperationOpenFilesByApp(const quint64 windowId,
                                       const QList<QUrl> urls,
                                       const QList<QString> apps,
                                       const QVariant custom,
                                       DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    bool handleOperationRenameFile(const quint64 windowId,
                                   const QUrl oldUrl,
                                   const QUrl newUrl);
    void handleOperationRenameFile(const quint64 windowId,
                                   const QUrl oldUrl,
                                   const QUrl newUrl,
                                   const QVariant custom,
                                   DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    bool handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, QString> pair,
                                    const bool replace);
    void handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, QString> pair,
                                    const bool replace,
                                    const QVariant custom,
                                    DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    bool handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair);
    void handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair,
                                    const QVariant custom,
                                    DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    QString handleOperationMkdir(const quint64 windowId,
                                 const QUrl url,
                                 const DFMBASE_NAMESPACE::Global::CreateFileType fileType);
    void handleOperationMkdir(const quint64 windowId,
                              const QUrl url,
                              const DFMBASE_NAMESPACE::Global::CreateFileType fileType,
                              const QVariant custom,
                              DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    bool handleOperationMkdir(const quint64 windowId,
                              const QUrl url);
    void handleOperationMkdir(const quint64 windowId,
                              const QUrl url,
                              const QVariant custom,
                              DFMBASE_NAMESPACE::Global::OperatorCallback callback);

    QString handleOperationTouchFile(const quint64 windowId,
                                     const QUrl url,
                                     const DFMBASE_NAMESPACE::Global::CreateFileType fileType,
                                     const QString suffix);
    void handleOperationTouchFile(const quint64 windowId,
                                  const QUrl url,
                                  const DFMBASE_NAMESPACE::Global::CreateFileType fileType,
                                  const QString suffix,
                                  const QVariant custom,
                                  DFMBASE_NAMESPACE::Global::OperatorCallback callbackImmediately);

    bool handleOperationLinkFile(const quint64 windowId,
                                 const QUrl url,
                                 const QUrl link);
    void handleOperationLinkFile(const quint64 windowId,
                                 const QUrl url,
                                 const QUrl link,
                                 const QVariant custom,
                                 DFMBASE_NAMESPACE::Global::OperatorCallback callback);
    bool handleOperationSetPermission(const quint64 windowId,
                                      const QUrl url,
                                      const QFileDevice::Permissions permissions);
    void handleOperationSetPermission(const quint64 windowId,
                                      const QUrl url,
                                      const QFileDevice::Permissions permissions,
                                      const QVariant custom,
                                      DFMBASE_NAMESPACE::Global::OperatorCallback callBack);
    bool handleOperationWriteToClipboard(const quint64 windowId,
                                         const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                         const QList<QUrl> urls);
    bool handleOperationWriteDataToClipboard(const quint64 windowId,
                                             QMimeData *data);
    bool handleOperationOpenInTerminal(const quint64 windowId,
                                       const QList<QUrl> urls);
    bool handleOperationSaveOperations(const QVariantMap values);
    bool handleOperationCleanSaveOperationsStack();
    bool handleOperationRevocation(const quint64 windowId,
                                   DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);
    bool handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls);
    void handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls,
                                  const QVariant custom, DFMBASE_NAMESPACE::Global::OperatorCallback callback);

private slots:
    void invokeRegister(const QString scheme, const FileOperationsFunctions functions);
    void invokeUnregister(const QString scheme);

private:
    enum class RenameTypes {
        kBatchRepalce,
        kBatchCustom,
        kBatchAppend
    };
    explicit FileOperationsEventReceiver(QObject *parent = nullptr);
    bool initService();
    QString newDocmentName(const QString targetdir,
                           const QString suffix,
                           const DFMBASE_NAMESPACE::Global::CreateFileType fileType);
    QString newDocmentName(QString targetdir,
                           const QString &baseName,
                           const QString &suffix);
    void initDBus();

    bool revocation(const quint64 windowId, const QVariantMap &ret,
                    DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle);

    bool doRenameFiles(const quint64 windowId, const QList<QUrl> urls,
                       const QPair<QString, QString> pair,
                       const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair2,
                       const RenameTypes type,
                       QMap<QUrl, QUrl> &successUrls, QString &errorMsg,
                       const QVariant custom = QVariant(), DFMBASE_NAMESPACE::Global::OperatorCallback callback = nullptr);
    JobHandlePointer doMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                   DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    JobHandlePointer doRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources,
                                        const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags, DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    JobHandlePointer doCopyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags, DFMGLOBAL_NAMESPACE::OperatorHandleCallback callbaskHandle);
    JobHandlePointer doCutFile(quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                               DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    JobHandlePointer doDeleteFile(const quint64 windowId, const QList<QUrl> sources,
                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags, DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    JobHandlePointer doCleanTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                  DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback);
    QString doTouchFilePremature(const quint64 windowId, const QUrl url,
                                 const DFMBASE_NAMESPACE::Global::CreateFileType fileType, const QString suffix,
                                 const QVariant custom, DFMBASE_NAMESPACE::Global::OperatorCallback callbackImmediately);
    bool doTouchFilePractically(const quint64 windowId, const QUrl url);
    void saveRenameOperate(const QString &sourcesUrl, const QString &targetUrl);

private:
    QSharedPointer<FileCopyMoveJob> copyMoveJob { nullptr };
    QSharedPointer<QMutex> getServiceMutex { nullptr };
    QPointer<DSC_NAMESPACE::FileOperationsService> operationsService { nullptr };
    DFMBASE_NAMESPACE::DialogManager *dialogManager { nullptr };
    QMap<QString, FileOperationsFunctions> functions;
    QSharedPointer<QMutex> functionsMutex { nullptr };
    QSharedPointer<OperationsStackManagerInterface> operationsStackDbus;
};

DPFILEOPERATIONS_END_NAMESPACE

Q_DECLARE_METATYPE(QSharedPointer<QList<QUrl>>)

#endif   // FILEOPERATIONSEVENTRECEIVER_H
