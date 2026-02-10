// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MYFILEPLUGIN_H
#define MYFILEPLUGIN_H

#include <dfm-extension/file/dfmextfileplugin.h>

namespace Exapmle {

class MyFilePlugin : public DFMEXT::DFMExtFilePlugin
{
public:
    MyFilePlugin();

    void fileOperationHooks(DFMEXT::DFMExtFileOperationHook *hook);
};

}   // namespace Exapmle

#endif   // MYFILEPLUGIN_H
