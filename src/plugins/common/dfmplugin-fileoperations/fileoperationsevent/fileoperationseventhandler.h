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
#ifndef FILEOPERATIONSEVENTHANDLER_H
#define FILEOPERATIONSEVENTHANDLER_H

#include "dfmplugin_fileoperations_global.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>

DPFILEOPERATIONS_BEGIN_NAMESPACE

class FileOperationsEventHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperationsEventHandler)

public:
    static FileOperationsEventHandler *instance();
    void handleJobResult(DFMBASE_NAMESPACE::AbstractJobHandler::JobType jobType, JobHandlePointer ptr);

private:
    explicit FileOperationsEventHandler(QObject *parent = nullptr);
    void publishJobResultEvent(DFMBASE_NAMESPACE::AbstractJobHandler::JobType jobType,
                               const QList<QUrl> &srcUrls,
                               const QList<QUrl> &destUrls, bool ok, const QString &errMsg);
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATIONSEVENTHANDLER_H
