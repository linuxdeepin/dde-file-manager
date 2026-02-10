// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCLEANTRASHFILESWORKER_H
#define DOCLEANTRASHFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>

DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class DoCleanTrashFilesWorker : public FileOperateBaseWorker
{
    friend class CleanTrashFiles;
    Q_OBJECT
    explicit DoCleanTrashFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoCleanTrashFilesWorker() override;

protected:
    bool doWork() override;
    void onUpdateProgress() override;
    bool statisticsFilesSize() override;
    bool initArgs() override;

protected:
    bool cleanAllTrashFiles();
    bool clearTrashFile(const FileInfoPointer &trashInfo);
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const bool isTo = false,
                                                           const QString &errorMsg = QString());

private:
    bool deleteFile(const QUrl &url);

private:
    QAtomicInteger<qint64> cleanTrashFilesCount { 0 };
    QString trashInfoPath;
    QString trashFilePath;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCLEANTRASHFILESWORKER_H
