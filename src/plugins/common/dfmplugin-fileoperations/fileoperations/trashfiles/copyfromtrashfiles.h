// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COPYFROMTRASHFILES_H
#define COPYFROMTRASHFILES_H

#include "fileoperations/fileoperationutils/abstractjob.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QThread>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class CopyFromTrashTrashFiles : private AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit CopyFromTrashTrashFiles(QObject *parent = nullptr);

public:
    ~CopyFromTrashTrashFiles() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // COPYFROMTRASHFILES_H
