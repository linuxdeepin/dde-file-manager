// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCOPYFROMTRASHFILESWORKER_H
#define DOCOPYFROMTRASHFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <dfm-io/dfile.h>

#include <QObject>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class DoCopyFromTrashFilesWorker : public FileOperateBaseWorker
{
    friend class CopyFromTrashTrashFiles;
    Q_OBJECT
    explicit DoCopyFromTrashFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoCopyFromTrashFilesWorker() override;

protected:
    bool doWork() override;
    bool statisticsFilesSize() override;
    bool initArgs() override;

protected:
    bool doOperate();
    DFileInfoPointer createParentDir(const DFileInfoPointer &trashInfo,
                                     const DFileInfoPointer &restoreInfo,
                                     bool *result);

private:
    QAtomicInteger<qint64> completeFilesCount { 0 };
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCOPYFROMTRASHFILESWORKER_H
