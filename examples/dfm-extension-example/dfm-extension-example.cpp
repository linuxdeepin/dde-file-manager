// SPDX-FileCopyrightText: 2023 - 2026 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <dfm-extension/dfm-extension.h>

#include "mymenuplugin.h"
#include "myemblemiconplugin.h"
#ifdef DFMEXT_INTERFACE_Window
#    include "mywindowplugin.h"
#endif
#ifdef DFMEXT_INTERFACE_File
#    include "myfileplugin.h"
#endif

// 右键菜单的扩展
static DFMEXT::DFMExtMenuPlugin *myMenu { nullptr };
// 角标的扩展
static DFMEXT::DFMExtEmblemIconPlugin *myEmblemIcon { nullptr };

// 窗口的扩展
#ifdef DFMEXT_INTERFACE_Window
static DFMEXT::DFMExtWindowPlugin *myWindow { nullptr };
#endif

// 本地文件操作的扩展
#ifdef DFMEXT_INTERFACE_File
static DFMEXT::DFMExtFilePlugin *myFile { nullptr };
#endif

extern "C" void dfm_extension_initialize()
{
    myMenu = new Exapmle::MyMenuPlugin;
    myEmblemIcon = new Exapmle::MyEmblemIconPlugin;
#ifdef DFMEXT_INTERFACE_Window
    myWindow = new Exapmle::MyWindowPlugin;
#endif

#ifdef DFMEXT_INTERFACE_File
    myFile = new Exapmle::MyFilePlugin;
#endif
}

extern "C" void dfm_extension_shutdown()
{
    delete myMenu;
    delete myEmblemIcon;
#ifdef DFMEXT_INTERFACE_Window
    delete myWindow;
#endif

#ifdef DFMEXT_INTERFACE_File
    delete myFile;
#endif
}

extern "C" DFMEXT::DFMExtMenuPlugin *dfm_extension_menu()
{
    return myMenu;
}

extern "C" DFMEXT::DFMExtEmblemIconPlugin *dfm_extension_emblem()
{
    return myEmblemIcon;
}

#ifdef DFMEXT_INTERFACE_Window
extern "C" DFMEXT::DFMExtWindowPlugin *dfm_extension_window()
{
    return myWindow;
}
#endif

#ifdef DFMEXT_INTERFACE_File
extern "C" DFMEXT::DFMExtFilePlugin *dfm_extension_file()
{
    return myFile;
}
#endif
