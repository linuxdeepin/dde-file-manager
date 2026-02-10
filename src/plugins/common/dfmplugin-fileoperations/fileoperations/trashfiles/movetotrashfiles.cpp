// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "movetotrashfiles.h"
#include "domovetotrashfilesworker.h"

DPFILEOPERATIONS_USE_NAMESPACE
MoveToTrashFiles::MoveToTrashFiles(QObject *parent)
    : AbstractJob(new DoMoveToTrashFilesWorker(), parent)
{
}

MoveToTrashFiles::~MoveToTrashFiles()
{
}
