// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "restoretrashfiles.h"
#include "dorestoretrashfilesworker.h"

DPFILEOPERATIONS_USE_NAMESPACE
RestoreTrashFiles::RestoreTrashFiles(QObject *parent)
    : AbstractJob(new DoRestoreTrashFilesWorker(), parent)
{
}

RestoreTrashFiles::~RestoreTrashFiles()
{
}
