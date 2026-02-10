// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTFILEOPERATIONHOOK_H
#define DFMEXTFILEOPERATIONHOOK_H

#include <dfm-extension/dfm-extension-global.h>

#include <functional>
#include <vector>
#include <string>

BEGEN_DFMEXT_NAMESPACE

class DFMExtFileOperationHookPrivate;
class DFMExtFileOperationHook
{
public:
    using OpenFilesFunc = std::function<bool(const std::vector<std::string> &srcPaths,
                                             std::vector<std::string> *ignorePaths)>;

public:
    DFMExtFileOperationHook(DFMExtFileOperationHookPrivate *);
    ~DFMExtFileOperationHook();

    void registerOpenFiles(const OpenFilesFunc &func);

private:
    DFMExtFileOperationHookPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTFILEOPERATIONHOOK_H
