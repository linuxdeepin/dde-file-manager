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
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"
#include "services/common/dialog/dialogservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include <QObject>
#include <QPointer>
#include <QFileDevice>

Q_DECLARE_METATYPE(QFileDevice::Permissions)
Q_DECLARE_METATYPE(QList<QUrl>)

DPFILEOPERATIONS_BEGIN_NAMESPACE
class FileOperationsUtils;

class FileOperationsEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperationsEventReceiver)

public:
    static FileOperationsEventReceiver *instance();

public slots:
    void handleOperationCopy(quint64 windowId, const QList<QUrl> sources, const QUrl target,
                             const dfmbase::AbstractJobHandler::JobFlags flags =
                                     dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    void handleOperationCut(quint64 windowId, const QList<QUrl> sources, const QUrl target,
                            const dfmbase::AbstractJobHandler::JobFlags flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    void handleOperationMoveToTrash(quint64 windowId, const QList<QUrl> sources,
                                    const dfmbase::AbstractJobHandler::JobFlags flags =
                                            dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    void handleOperationRestoreFromTrash(quint64 windowId, const QList<QUrl> sources,
                                         const dfmbase::AbstractJobHandler::JobFlags flags =
                                                 dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    void handleOperationDeletes(quint64 windowId, const QList<QUrl> sources,
                                const dfmbase::AbstractJobHandler::JobFlags flags =
                                        dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    void handleOperationOpenFiles(quint64 windowId, QList<QUrl> urls);
    void handleOperationOpenFilesByApp(quint64 windowId, QList<QUrl> urls, QList<QString> apps);
    void handleOperationRenameFile(quint64 windowId, QUrl oldUrl, QUrl newUrl);
    void handleOperationMkdir(quint64 windowId, QUrl url);
    void handleOperationTouchFile(quint64 windowId, QUrl url);
    void handleOperationLinkFile(quint64 windowId, QUrl url, QUrl link);
    void handleOperationSetPermission(quint64 windowId, QUrl url, QFileDevice::Permissions permissions);
    void handleOperationSetWriteToClipboard(quint64 windowId, dfmbase::ClipBoard::ClipboardAction action, QList<QUrl> urls);

private:
    explicit FileOperationsEventReceiver(QObject *parent = nullptr);
    bool initService();
    bool getDialogService();

private:
    QSharedPointer<FileOperationsUtils> copyMoveUtils { nullptr };
    QSharedPointer<QMutex> getServiceMutex { nullptr };
    QPointer<DSC_NAMESPACE::FileOperationsService> operationsService { nullptr };
    QPointer<DSC_NAMESPACE::DialogService> dialogService { nullptr };
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATIONSEVENTRECEIVER_H
