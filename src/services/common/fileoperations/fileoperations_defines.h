/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
#ifndef FILEOPERATIONS_DEFINES_H
#define FILEOPERATIONS_DEFINES_H

#include "dfm_common_service_global.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QObject>
#include <QUrl>
#include <QFileDevice>
#include <QSharedPointer>

#include <functional>

DSC_BEGIN_NAMESPACE

namespace FileOperationsSpace {
using handleOperationPaste = std::function<JobHandlePointer(const quint64 windowId,
                                                            const QList<QUrl> sources,
                                                            const QUrl target,
                                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using handleOperationCut = std::function<JobHandlePointer(const quint64 windowId,
                                                          const QList<QUrl> sources,
                                                          const QUrl target,
                                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using handleOperationMoveToTrash = std::function<JobHandlePointer(const quint64 windowId,
                                                                  const QList<QUrl> sources,
                                                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using handleOperationRestoreFromTrash = std::function<JobHandlePointer(const quint64 windowId,
                                                                       const QList<QUrl> sources,
                                                                       const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using handleOperationDeletes = std::function<JobHandlePointer(const quint64 windowId,
                                                              const QList<QUrl> sources,
                                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using handleOperationOpenFiles = std::function<bool(const quint64 windowId,
                                                    const QList<QUrl> urls,
                                                    QString *)>;

using handleOperationOpenFilesByApp = std::function<bool(const quint64 windowId,
                                                         const QList<QUrl> urls,
                                                         const QList<QString> apps,
                                                         QString *)>;

using handleOperationRenameFile = std::function<bool(const quint64 windowId,
                                                     const QUrl oldUrl,
                                                     const QUrl newUrl,
                                                     QString *)>;

using handleOperationMkdir = std::function<bool(const quint64 windowId,
                                                const QUrl url,
                                                QString *)>;
using handleOperationTouchFile = std::function<bool(const quint64 windowId,
                                                    const QUrl url,
                                                    QString *)>;
using handleOperationLinkFile = std::function<bool(const quint64 windowId,
                                                   const QUrl url,
                                                   const QUrl link,
                                                   QString *)>;
using handleOperationSetPermission = std::function<bool(const quint64 windowId,
                                                        const QUrl url,
                                                        const QFileDevice::Permissions permissions,
                                                        QString *)>;
using handleOperationCopy = std::function<bool(const quint64 windowId,
                                               const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                               const QList<QUrl> urls)>;
using handleOperationOpenInTerminal = std::function<bool(const quint64 windowId,
                                                         const QUrl url,
                                                         QString *)>;

struct FileOperationsInfo
{
    handleOperationPaste paste { nullptr };
    handleOperationCut cut { nullptr };
    handleOperationMoveToTrash moveToTash { nullptr };
    handleOperationRestoreFromTrash restoreFromTrash { nullptr };
    handleOperationDeletes deletes { nullptr };
    handleOperationOpenFiles openFiles { nullptr };
    handleOperationOpenFilesByApp openFilesByApp { nullptr };
    handleOperationRenameFile renameFile { nullptr };
    handleOperationMkdir makedir { nullptr };
    handleOperationTouchFile touchFile { nullptr };
    handleOperationLinkFile linkFile { nullptr };
    handleOperationSetPermission setPermission { nullptr };
    handleOperationCopy copy { nullptr };
    handleOperationOpenInTerminal openInTerminal { nullptr };
};

};   // namespace FileOperationsSpace

DSC_END_NAMESPACE

using FileOperationsFunctions = QSharedPointer<DSC_NAMESPACE::FileOperationsSpace::FileOperationsInfo>;
Q_DECLARE_METATYPE(FileOperationsFunctions)

#endif   // FILEOPERATIONS_DEFINES_H
