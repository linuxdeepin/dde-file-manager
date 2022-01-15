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
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"
#include "services/common/dialog/dialogservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"
#include "services/common/fileoperations/fileoperations_defines.h"

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
    JobHandlePointer handleOperationPaste(const quint64 windowId,
                                          const QList<QUrl> sources,
                                          const QUrl target,
                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    JobHandlePointer handleOperationCut(const quint64 windowId,
                                        const QList<QUrl> sources,
                                        const QUrl target,
                                        const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    JobHandlePointer handleOperationMoveToTrash(const quint64 windowId,
                                                const QList<QUrl> sources,
                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    JobHandlePointer handleOperationRestoreFromTrash(const quint64 windowId,
                                                     const QList<QUrl> sources,
                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    JobHandlePointer handleOperationDeletes(const quint64 windowId,
                                            const QList<QUrl> sources,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    void handleOperationPaste(const quint64 windowId,
                              const QList<QUrl> sources,
                              const QUrl target,
                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                              DFMBASE_NAMESPACE::Global::CopyMoveFileCallback callback);
    void handleOperationCut(const quint64 windowId,
                            const QList<QUrl> sources,
                            const QUrl target,
                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                            DFMBASE_NAMESPACE::Global::CopyMoveFileCallback callback);
    void handleOperationMoveToTrash(const quint64 windowId,
                                    const QList<QUrl> sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                    DFMBASE_NAMESPACE::Global::CopyMoveFileCallback callback);
    void handleOperationRestoreFromTrash(const quint64 windowId,
                                         const QList<QUrl> sources,
                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                         DFMBASE_NAMESPACE::Global::CopyMoveFileCallback callback);
    void handleOperationDeletes(const quint64 windowId,
                                const QList<QUrl> sources,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                DFMBASE_NAMESPACE::Global::CopyMoveFileCallback callback);
    bool handleOperationOpenFiles(const quint64 windowId,
                                  const QList<QUrl> urls);
    void handleOperationOpenFiles(const quint64 windowId,
                                  const QList<QUrl> urls,
                                  DFMBASE_NAMESPACE::Global::OpenFilesCallback callback);
    bool handleOperationOpenFilesByApp(const quint64 windowId,
                                       const QList<QUrl> urls,
                                       const QList<QString> apps);
    void handleOperationOpenFilesByApp(const quint64 windowId,
                                       const QList<QUrl> urls,
                                       const QList<QString> apps,
                                       DFMBASE_NAMESPACE::Global::OpenFilesCallback callback);
    bool handleOperationRenameFile(const quint64 windowId,
                                   const QUrl oldUrl,
                                   const QUrl newUrl);
    void handleOperationRenameFile(const quint64 windowId,
                                   const QUrl oldUrl,
                                   const QUrl newUrl,
                                   DFMBASE_NAMESPACE::Global::RenameFileCallback callback);
    QString handleOperationMkdir(const quint64 windowId,
                                 const QUrl url,
                                 const DFMBASE_NAMESPACE::GlobalCreateFileType fileType);
    void handleOperationMkdir(const quint64 windowId,
                              const QUrl url,
                              const DFMBASE_NAMESPACE::GlobalCreateFileType fileType,
                              DFMBASE_NAMESPACE::Global::CreateFileCallback callBack);
    bool handleOperationMkdir(const quint64 windowId,
                              const QUrl url);
    void handleOperationMkdir(const quint64 windowId,
                              const QUrl url,
                              DFMBASE_NAMESPACE::Global::CreateFileCallback callBack);
    bool handleOperationTouchFile(const quint64 windowId,
                                  const QUrl url);
    void handleOperationTouchFile(const quint64 windowId,
                                  const QUrl url,
                                  DFMBASE_NAMESPACE::Global::CreateFileCallback callBack);
    QString handleOperationTouchFile(const quint64 windowId,
                                     const QUrl url,
                                     const DFMBASE_NAMESPACE::GlobalCreateFileType fileType);
    void handleOperationTouchFile(const quint64 windowId,
                                  const QUrl url,
                                  const DFMBASE_NAMESPACE::GlobalCreateFileType fileType,
                                  DFMBASE_NAMESPACE::Global::CreateFileCallback callBack);
    bool handleOperationLinkFile(const quint64 windowId,
                                 const QUrl url,
                                 const QUrl link);
    void handleOperationLinkFile(const quint64 windowId,
                                 const QUrl url,
                                 const QUrl link,
                                 DFMBASE_NAMESPACE::Global::LinkFileCallback callback);
    bool handleOperationSetPermission(const quint64 windowId,
                                      const QUrl url,
                                      const QFileDevice::Permissions permissions);
    void handleOperationSetPermission(const quint64 windowId,
                                      const QUrl url,
                                      const QFileDevice::Permissions permissions,
                                      DFMBASE_NAMESPACE::Global::SetFilePermissionCallback callBack);
    bool handleOperationCopy(const quint64 windowId,
                             const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                             const QList<QUrl> urls);
private slots:
    void invokeRegister(const QString scheme, const FileOperationsFunctions functions);
    void invokeUnregister(const QString scheme);

private:
    explicit FileOperationsEventReceiver(QObject *parent = nullptr);
    bool initService();
    bool getDialogService();
    QString newDocmentName(const QString targetdir,
                           const DFMBASE_NAMESPACE::GlobalCreateFileType fileType);
    QString newDocmentName(QString targetdir,
                           const QString &baseName,
                           const QString &suffix);

private:
    QSharedPointer<FileCopyMoveJob> copyMoveJob { nullptr };
    QSharedPointer<QMutex> getServiceMutex { nullptr };
    QPointer<DSC_NAMESPACE::FileOperationsService> operationsService { nullptr };
    QPointer<DSC_NAMESPACE::DialogService> dialogService { nullptr };
    QMap<QString, FileOperationsFunctions> functions;
    QSharedPointer<QMutex> functionsMutex { nullptr };
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATIONSEVENTRECEIVER_H
