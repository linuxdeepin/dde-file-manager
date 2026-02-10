// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELETEFILES_H
#define DELETEFILES_H

#include "fileoperations/fileoperationutils/abstractjob.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QThread>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class DeleteFiles : public AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit DeleteFiles(QObject *parent = nullptr);

public:
    ~DeleteFiles() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DELETEFILES_H
