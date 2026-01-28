// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCOPYFILESWORKER_H
#define DOCOPYFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/threadcontainer.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-io/dfile.h>

#include <QObject>
#include <QMutex>
#include <QTime>
#include <QQueue>
#include <QThread>
#include <QThreadPool>
#include <QMultiMap>

DPFILEOPERATIONS_BEGIN_NAMESPACE
USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
class DoCopyFilesWorker : public FileOperateBaseWorker
{
    friend class CopyFiles;
    Q_OBJECT
    explicit DoCopyFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoCopyFilesWorker() override;

protected:
    bool doWork() override;
    void stop() override;
    bool initArgs() override;
    void endWork() override;

protected:
    bool copyFiles();

private slots:
    void onUpdateProgress() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCOPYFILESWORKER_H
