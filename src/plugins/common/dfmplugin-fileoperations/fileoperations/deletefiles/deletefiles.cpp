// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deletefiles.h"
#include "dodeletefilesworker.h"

DPFILEOPERATIONS_USE_NAMESPACE
DeleteFiles::DeleteFiles(QObject *parent)
    : AbstractJob(new DoDeleteFilesWorker(), parent)
{
}

DeleteFiles::~DeleteFiles()
{
}
