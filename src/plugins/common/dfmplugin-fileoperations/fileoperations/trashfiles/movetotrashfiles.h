// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOVETOtRASHFILES_H
#define MOVETOtRASHFILES_H

#include "fileoperations/fileoperationutils/abstractjob.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QThread>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class MoveToTrashFiles : private AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit MoveToTrashFiles(QObject *parent = nullptr);

public:
    ~MoveToTrashFiles() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // MOVETOtRASHFILES_H
