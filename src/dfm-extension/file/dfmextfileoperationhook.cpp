// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "private/dfmextfileoperationhookprivate.h"

#include <dfm-extension/file/dfmextfileoperationhook.h>

#include <cassert>

BEGEN_DFMEXT_NAMESPACE

DFMExtFileOperationHook::DFMExtFileOperationHook(DFMExtFileOperationHookPrivate *d_ptr)
    : d(d_ptr)
{
    assert(d);
}

DFMExtFileOperationHook::~DFMExtFileOperationHook()
{
    delete d;
}

void DFMExtFileOperationHook::registerOpenFiles(const OpenFilesFunc &func)
{
    d->openFilesFunc = func;
}

END_DFMEXT_NAMESPACE
