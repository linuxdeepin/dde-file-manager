#include <dfm-extension/dfm-extension.h>

#include "mymenuplugin.h"
#include "myemblemiconplugin.h"
#include "mywindowplugin.h"

// 右键菜单的扩展
static DFMEXT::DFMExtMenuPlugin *myMenu { nullptr };
// 角标的扩展
static DFMEXT::DFMExtEmblemIconPlugin *myEmblemIcon { nullptr };
// 窗口的扩展
static DFMEXT::DFMExtWindowPlugin *myWindow { nullptr };

extern "C" void dfm_extension_initiliaze()
{
    myMenu = new Exapmle::MyMenuPlugin;
    myEmblemIcon = new Exapmle::MyEmblemIconPlugin;
    myWindow = new Exapmle::MyWindowPlugin;
}

extern "C" void dfm_extension_shutdown()
{
    delete myMenu;
    delete myEmblemIcon;
    delete myWindow;
}

extern "C" DFMEXT::DFMExtMenuPlugin *dfm_extension_menu()
{
    return myMenu;
}

extern "C" DFMEXT::DFMExtEmblemIconPlugin *dfm_extension_emblem()
{
    return myEmblemIcon;
}

#ifdef DFMEXT_VERSION
#    if (DFMEXT_VERSION >= DFMEXT_VERSION_CHECK(6, 0, 0))
extern "C" DFMEXT::DFMExtWindowPlugin *dfm_extension_window()
{
    return myWindow;
}
#    endif
#endif
