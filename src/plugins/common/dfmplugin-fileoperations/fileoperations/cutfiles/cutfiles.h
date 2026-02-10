// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUTFILES_H
#define CUTFILES_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractjob.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QThread>

DPFILEOPERATIONS_BEGIN_NAMESPACE
class CutFiles : public AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit CutFiles(QObject *parent = nullptr);

public:
    ~CutFiles() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // CUTFILES_H
