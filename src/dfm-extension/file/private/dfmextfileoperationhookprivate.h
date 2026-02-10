// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTFILEOPERATIONHOOKPRIVATE_H
#define DFMEXTFILEOPERATIONHOOKPRIVATE_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/file/dfmextfileoperationhook.h>

BEGEN_DFMEXT_NAMESPACE

class DFMExtFileOperationHookPrivate
{
public:
    DFMExtFileOperationHookPrivate();

    DFMExtFileOperationHook::OpenFilesFunc openFilesFunc { nullptr };
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTFILEOPERATIONHOOKPRIVATE_H
