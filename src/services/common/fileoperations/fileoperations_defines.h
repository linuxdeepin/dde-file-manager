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
using HandleOperationCopy = std::function<JobHandlePointer(const quint64 windowId,
                                                           const QList<QUrl> sources,
                                                           const QUrl target,
                                                           const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using HandleOperationCut = std::function<JobHandlePointer(const quint64 windowId,
                                                          const QList<QUrl> sources,
                                                          const QUrl target,
                                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using HandleOperationMoveToTrash = std::function<JobHandlePointer(const quint64 windowId,
                                                                  const QList<QUrl> sources,
                                                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using HandleOperationRestoreFromTrash = std::function<JobHandlePointer(const quint64 windowId,
                                                                       const QList<QUrl> sources,
                                                                       const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using HandleOperationDeletes = std::function<JobHandlePointer(const quint64 windowId,
                                                              const QList<QUrl> sources,
                                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)>;
using HandleOperationOpenFiles = std::function<bool(const quint64 windowId,
                                                    const QList<QUrl> urls,
                                                    QString *)>;

using HandleOperationOpenFilesByApp = std::function<bool(const quint64 windowId,
                                                         const QList<QUrl> urls,
                                                         const QList<QString> apps,
                                                         QString *)>;

using HandleOperationRenameFile = std::function<bool(const quint64 windowId,
                                                     const QUrl oldUrl,
                                                     const QUrl newUrl,
                                                     QString *)>;

using HandleOperationMkdir = std::function<bool(const quint64 windowId,
                                                const QUrl url,
                                                QString *)>;
using HandleOperationTouchFile = std::function<bool(const quint64 windowId,
                                                    const QUrl url,
                                                    QString *)>;
using HandleOperationLinkFile = std::function<bool(const quint64 windowId,
                                                   const QUrl url,
                                                   const QUrl link,
                                                   QString *)>;
using HandleOperationSetPermission = std::function<bool(const quint64 windowId,
                                                        const QUrl url,
                                                        const QFileDevice::Permissions permissions,
                                                        QString *)>;
using HandleOperationWriteUrlsToClipboard = std::function<bool(const quint64 windowId,
                                                               const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                                               const QList<QUrl> urls)>;
using HandleOperationOpenInTerminal = std::function<bool(const quint64 windowId,
                                                         const QList<QUrl> urls,
                                                         QString *)>;
using HandleOperationCleanTrash = std::function<JobHandlePointer(const quint64 windowId,
                                                                 const QList<QUrl> sources)>;
using HandleOperationWriteDataToClipboard = std::function<JobHandlePointer(const quint64 windowId,
                                                                           const QMimeData *data)>;

struct FileOperationsInfo
{
    HandleOperationCopy copy { nullptr };
    HandleOperationCut cut { nullptr };
    HandleOperationMoveToTrash moveToTash { nullptr };
    HandleOperationRestoreFromTrash restoreFromTrash { nullptr };
    HandleOperationDeletes deletes { nullptr };
    HandleOperationOpenFiles openFiles { nullptr };
    HandleOperationOpenFilesByApp openFilesByApp { nullptr };
    HandleOperationRenameFile renameFile { nullptr };
    HandleOperationMkdir makedir { nullptr };
    HandleOperationTouchFile touchFile { nullptr };
    HandleOperationLinkFile linkFile { nullptr };
    HandleOperationSetPermission setPermission { nullptr };
    HandleOperationWriteUrlsToClipboard writeUrlsToClipboard { nullptr };
    HandleOperationOpenInTerminal openInTerminal { nullptr };
    HandleOperationCleanTrash cleanTrash { nullptr };
    HandleOperationWriteDataToClipboard writeDataToClipboard { nullptr };   // TODO(lanxuesong): impl me!
};

};   // namespace FileOperationsSpace

DSC_END_NAMESPACE

using FileOperationsFunctions = QSharedPointer<DSC_NAMESPACE::FileOperationsSpace::FileOperationsInfo>;
Q_DECLARE_METATYPE(FileOperationsFunctions)

#endif   // FILEOPERATIONS_DEFINES_H
