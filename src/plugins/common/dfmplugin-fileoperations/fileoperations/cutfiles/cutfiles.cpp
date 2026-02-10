// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cutfiles.h"
#include "docutfilesworker.h"

DPFILEOPERATIONS_USE_NAMESPACE
CutFiles::CutFiles(QObject *parent)
    : AbstractJob(new DoCutFilesWorker(), parent)
{
}

CutFiles::~CutFiles()
{
}
