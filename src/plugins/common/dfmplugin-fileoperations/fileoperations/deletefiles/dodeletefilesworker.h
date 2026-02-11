// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DODELETEFILESWORKER_H
#define DODELETEFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>

DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class DoDeleteFilesWorker : public AbstractWorker
{
    friend class DeleteFiles;
    Q_OBJECT
    explicit DoDeleteFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoDeleteFilesWorker() override;

protected:
    bool doWork() override;
    void stop() override;
    void onUpdateProgress() override;

protected:
    bool deleteAllFiles();
    bool deleteFilesOnCanNotRemoveDevice();
    bool deleteFilesOnOtherDevice();
    bool deleteFileOnOtherDevice(const QUrl &url);
    bool deleteDirOnOtherDevice(const FileInfoPointer &dir);
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const QString &errorMsg = QString());

private:
    QAtomicInteger<qint64> deleteFilesCount { 0 };
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DODELETEFILESWORKER_H
