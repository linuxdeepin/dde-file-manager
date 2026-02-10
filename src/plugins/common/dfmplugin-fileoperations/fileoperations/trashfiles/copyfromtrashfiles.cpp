// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "copyfromtrashfiles.h"
#include "docopyfromtrashfilesworker.h"

DPFILEOPERATIONS_USE_NAMESPACE
CopyFromTrashTrashFiles::CopyFromTrashTrashFiles(QObject *parent)
    : AbstractJob(new DoCopyFromTrashFilesWorker(), parent)
{
}

CopyFromTrashTrashFiles::~CopyFromTrashTrashFiles()
{
}
