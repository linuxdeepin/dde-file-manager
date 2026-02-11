// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COPYFILES_H
#define COPYFILES_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractjob.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QThread>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class DoCopyFilesWorker;
class CopyFiles : public AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit CopyFiles(QObject *parent = nullptr);

public:
    ~CopyFiles() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // COPYFILES_H
