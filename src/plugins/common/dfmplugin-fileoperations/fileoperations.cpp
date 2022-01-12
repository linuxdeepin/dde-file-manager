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
#include "fileoperationseventreceiver.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_event_defines.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

void FileOperations::initialize()
{
    initEventHandle();
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
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kPasteFile,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationCopy);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCutFile,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationCut);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kRestoreFromTrash,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationRestoreFromTrash);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMoveToTrash,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationMoveToTrash);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kDeleteFiles,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationDeletes);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenFiles,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationOpenFiles);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenFilesByApp,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationOpenFilesByApp);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kRenameFile,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationRenameFile);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kMkdir,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationMkdir);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kTouchFile,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationTouchFile);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kCreateSymlink,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationLinkFile);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kSetPermission,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationSetPermission);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kWriteUrlsToClipboard,
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleOperationSetWriteToClipboard);
}
