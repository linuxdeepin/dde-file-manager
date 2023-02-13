// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cleantrashfiles.h"
#include "docleantrashfilesworker.h"

DPFILEOPERATIONS_USE_NAMESPACE
CleanTrashFiles::CleanTrashFiles(QObject *parent)
    : AbstractJob(new DoCleanTrashFilesWorker(), parent)
{
}

CleanTrashFiles::~CleanTrashFiles()
{
}
