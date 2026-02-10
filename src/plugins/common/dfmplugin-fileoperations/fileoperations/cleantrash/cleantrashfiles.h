// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLEANTRASHFILES_H
#define CLEANTRASHFILES_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractjob.h"
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QThread>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class CleanTrashFiles : public AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit CleanTrashFiles(QObject *parent = nullptr);

public:
    ~CleanTrashFiles() override;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // CLEANTRASHFILES_H
