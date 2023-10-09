// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperations.h"
#include "fileoperationsevent/fileoperationseventreceiver.h"
#include "fileoperationsevent/trashfileeventreceiver.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

void FileOperations::initialize()
{
    initEventHandle();
}

bool FileOperations::start()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.operations", &err);
    if (!ret)
        qWarning() << "create dconfig failed: " << err;

    return true;
}

/*!
 * \brief FileOperations::initEventHandle Initialize all event handling
 */
void FileOperations::initEventHandle()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopy,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&FileOperationsEventReceiver::handleOperationCopy));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&FileOperationsEventReceiver::handleOperationCut));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRestoreFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationRestoreFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationCopyFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationCopyFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationMoveToTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&FileOperationsEventReceiver::handleOperationDeletes));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCleanTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationCleanTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopy,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationCopy));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationCut));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRestoreFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationRestoreFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationMoveToTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationDeletes));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCleanTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationCleanTrash));

    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     bool *ok)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFilesByApp,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QList<QString>)>(&FileOperationsEventReceiver::handleOperationOpenFilesByApp));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFilesByApp,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QList<QString>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationOpenFilesByApp));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags)>(&FileOperationsEventReceiver::handleOperationRenameFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationRenameFile));

    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, QString>,
                                                                                     const bool replace)>(&FileOperationsEventReceiver::handleOperationRenameFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, QString>,
                                                                                     const bool replace,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationRenameFiles));

    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, AbstractJobHandler::FileNameAddFlag>)>(&FileOperationsEventReceiver::handleOperationRenameFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, AbstractJobHandler::FileNameAddFlag>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationRenameFiles));

    dpfSignalDispatcher->subscribe(GlobalEventType::kMkdir,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64, const QUrl)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfSignalDispatcher->subscribe(GlobalEventType::kMkdir,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QUrl,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<QString (FileOperationsEventReceiver::*)(const quint64,
                                                                                        const QUrl,
                                                                                        const CreateFileType,
                                                                                        const QString)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const CreateFileType,
                                                                                     const QString,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationTouchFile));

    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<QString (FileOperationsEventReceiver::*)(const quint64,
                                                                                        const QUrl,
                                                                                        const QUrl,
                                                                                        const QString)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const QString,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCreateSymlink,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const bool force,
                                                                                     const bool silence)>(&FileOperationsEventReceiver::handleOperationLinkFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCreateSymlink,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const bool force,
                                                                                     const bool silence,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationLinkFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kSetPermission,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QFileDevice::Permissions)>(&FileOperationsEventReceiver::handleOperationSetPermission));
    dpfSignalDispatcher->subscribe(GlobalEventType::kSetPermission,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QFileDevice::Permissions,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationSetPermission));
    dpfSignalDispatcher->subscribe(GlobalEventType::kWriteUrlsToClipboard,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationWriteToClipboard);
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenInTerminal,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationOpenInTerminal);
    dpfSignalDispatcher->subscribe(GlobalEventType::kWriteCustomToClipboard,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationWriteDataToClipboard);
    dpfSignalDispatcher->subscribe(GlobalEventType::kSaveOperator,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationSaveOperations);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCleanSaveOperator,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationCleanSaveOperationsStack);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRevocation,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationRevocation);
    dpfSignalDispatcher->subscribe(GlobalEventType::kHideFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>)>(&FileOperationsEventReceiver::handleOperationHideFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kHideFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationHideFiles));
}
