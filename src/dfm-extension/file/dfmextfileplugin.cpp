// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <dfm-extension/file/dfmextfileplugin.h>

BEGEN_DFMEXT_NAMESPACE

class DFMExtFilePluginPrivate
{
public:
    DFMExtFilePlugin::FileOperationHooksFunc hooksFunc;
};

DFMExtFilePlugin::DFMExtFilePlugin()
    : d(new DFMExtFilePluginPrivate)
{
}

DFMExtFilePlugin::~DFMExtFilePlugin()
{
    delete d;
}

void DFMExtFilePlugin::fileOperationHooks(DFMExtFileOperationHook *hook)
{
    if (d->hooksFunc)
        d->hooksFunc(hook);
}

void DFMExtFilePlugin::registerFileOperationHooks(const FileOperationHooksFunc &func)
{
    d->hooksFunc = func;
}

END_DFMEXT_NAMESPACE
