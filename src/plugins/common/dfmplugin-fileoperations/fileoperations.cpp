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
#include "fileoperations.h"
#include "fileoperationsevent/fileoperationseventreceiver.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dfm_global_defines.h"

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

void FileOperations::initialize()
{
    initEventHandle();
    FileOperationsEventReceiver::instance()->connectService();
}

bool FileOperations::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag FileOperations::stop()
{
    return kSync;
}
/*!
 * \brief FileOperations::initEventHandle Initialize all event handling
 */
void FileOperations::initEventHandle()
{
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCopy,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<JobHandlePointer (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                                          const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags)>(&FileOperationsEventReceiver::handleOperationCopy));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCutFile,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<JobHandlePointer (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                                          const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags)>(&FileOperationsEventReceiver::handleOperationCut));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kRestoreFromTrash,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<JobHandlePointer (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags)>(&FileOperationsEventReceiver::handleOperationRestoreFromTrash));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMoveToTrash,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<JobHandlePointer (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags)>(&FileOperationsEventReceiver::handleOperationMoveToTrash));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kDeleteFiles,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<JobHandlePointer (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags)>(&FileOperationsEventReceiver::handleOperationDeletes));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCleanTrash,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<JobHandlePointer (FileOperationsEventReceiver::*)(const quint64,
                                                                                                          const QList<QUrl>)>(&FileOperationsEventReceiver::handleOperationCleanTrash));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCopy,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                              const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationCopy));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCutFile,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                              const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationCut));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kRestoreFromTrash,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationRestoreFromTrash));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMoveToTrash,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationMoveToTrash));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kDeleteFiles,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationDeletes));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCleanTrash,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationCleanTrash));

    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenFiles,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QList<QUrl>,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenFiles,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QList<QUrl>)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenFilesByApp,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QList<QUrl>,
                                                                                              const QList<QString>)>(&FileOperationsEventReceiver::handleOperationOpenFilesByApp));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenFilesByApp,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QList<QUrl>,
                                                                                              const QList<QString>,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationOpenFilesByApp));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kRenameFile,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QUrl,
                                                                                              const QUrl)>(&FileOperationsEventReceiver::handleOperationRenameFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kRenameFile,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QUrl,
                                                                                              const QUrl,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationRenameFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMkdir,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<QString (FileOperationsEventReceiver::*)(const quint64, const QUrl, const CreateFileType)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMkdir,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<bool (FileOperationsEventReceiver::*)(const quint64, const QUrl)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMkdirCallBack,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QUrl,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMkdirCallBack,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QUrl, const CreateFileType,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kTouchFile,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<QString (FileOperationsEventReceiver::*)(const quint64,
                                                                                                 const QUrl,
                                                                                                 const CreateFileType,
                                                                                                 const QString)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kTouchFile,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<bool (FileOperationsEventReceiver::*)(const quint64, const QUrl)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kTouchFileCallBack,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QUrl,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kTouchFileCallBack,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QUrl,
                                                                                              const CreateFileType,
                                                                                              const QString,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCreateSymlink,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QUrl,
                                                                                              const QUrl)>(&FileOperationsEventReceiver::handleOperationLinkFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCreateSymlink,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QUrl,
                                                                                              const QUrl,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationLinkFile));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kSetPermission,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QUrl,
                                                                                              const QFileDevice::Permissions)>(&FileOperationsEventReceiver::handleOperationSetPermission));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kSetPermission,
                                            FileOperationsEventReceiver::instance(),
                                            static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                              const QUrl,
                                                                                              const QFileDevice::Permissions,
                                                                                              const QVariant,
                                                                                              OperaterCallback)>(&FileOperationsEventReceiver::handleOperationSetPermission));
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kWriteUrlsToClipboard,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationWriteToClipboard);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenInTerminal,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationOpenInTerminal);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kWriteCustomToClipboard,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationWriteDataToClipboard);
}
