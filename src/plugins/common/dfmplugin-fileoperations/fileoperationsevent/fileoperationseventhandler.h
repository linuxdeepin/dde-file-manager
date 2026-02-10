// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

public Q_SLOTS:
    void handleErrorNotify(const JobInfoPointer &jobInfo);
    void handleFinishedNotify(const JobInfoPointer &jobInfo);

private:
    explicit FileOperationsEventHandler(QObject *parent = nullptr);
    void publishJobResultEvent(DFMBASE_NAMESPACE::AbstractJobHandler::JobType jobType,
                               const QList<QUrl> &srcUrls,
                               const QList<QUrl> &destUrls,
                               const QVariantList &customInfos,
                               bool ok, const QString &errMsg);
    void removeUrlsInClipboard(DFMBASE_NAMESPACE::AbstractJobHandler::JobType jobType,
                               const QList<QUrl> &srcUrls,
                               const QList<QUrl> &destUrls,
                               bool ok);
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATIONSEVENTHANDLER_H
