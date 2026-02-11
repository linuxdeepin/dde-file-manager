// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "copyfiles.h"
#include "docopyfilesworker.h"

#include <QDebug>

DPFILEOPERATIONS_USE_NAMESPACE
CopyFiles::CopyFiles(QObject *parent)
    : AbstractJob(new DoCopyFilesWorker(), parent)
{
}
CopyFiles::~CopyFiles()
{
}
