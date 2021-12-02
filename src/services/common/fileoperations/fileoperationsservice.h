/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef FILEOPERATIONSSERVICE_H
#define FILEOPERATIONSSERVICE_H

#include "dfm_common_service_global.h"
#include "dfm-framework/service/pluginservicecontext.h"
#include "dfm-base/base/abstractjobhandler.h"

DSC_BEGIN_NAMESPACE
class FileOperationsService final : public dpf::PluginService,
                                    dpf::AutoServiceRegister<FileOperationsService>
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperationsService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name() { return "org.deepin.service.FileOperationsService"; }

    JobHandlePointer copy(const QList<QUrl> &sources, const QUrl &target,
                          const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer moveToTrash(const QList<QUrl> &sources,
                                 const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer restoreFromTrash(const QList<QUrl> &sources,
                                      const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer deletes(const QList<QUrl> &sources,
                             const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer cut(const QList<QUrl> &sources, const QUrl &target,
                         const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);

private:
    explicit FileOperationsService(QObject *parent = nullptr);
    virtual ~FileOperationsService() override;
    static QList<JobHandlePointer> copyTask;
};

DSC_END_NAMESPACE

#endif   // FILEOPERATIONSSERVICE_H
