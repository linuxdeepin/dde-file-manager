// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESTORETRASHFILES_H
#define RESTORETRASHFILES_H

#include "fileoperations/fileoperationutils/abstractjob.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QThread>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class RestoreTrashFiles : private AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit RestoreTrashFiles(QObject *parent = nullptr);

public:
    ~RestoreTrashFiles() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // RESTORETRASHFILES_H
