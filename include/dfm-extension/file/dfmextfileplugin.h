// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTFILEPLUGIN_H
#define DFMEXTFILEPLUGIN_H

#include <dfm-extension/dfm-extension-global.h>

#include <functional>

BEGEN_DFMEXT_NAMESPACE

class DFMExtFileOperationHook;
class DFMExtFilePluginPrivate;
class DFMExtFilePlugin
{
    DFM_DISABLE_COPY(DFMExtFilePlugin)

public:
    using FileOperationHooksFunc = std::function<void(DFMExtFileOperationHook *)>;

public:
    DFMExtFilePlugin();
    ~DFMExtFilePlugin();

    DFM_FAKE_VIRTUAL void fileOperationHooks(DFMExtFileOperationHook *hook);

public:
    void registerFileOperationHooks(const FileOperationHooksFunc &func);

private:
    DFMExtFilePluginPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTFILEPLUGIN_H
