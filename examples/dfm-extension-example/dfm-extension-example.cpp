#include <dfm-extension/dfm-extension.h>

#include "mymenuplugin.h"
#include "myemblemiconplugin.h"
#include "mywindowplugin.h"

// 右键菜单的扩展
static DFMEXT::DFMExtMenuPlugin *myMenu { nullptr };
// 角标的扩展
static DFMEXT::DFMExtEmblemIconPlugin *myEmblemIcon { nullptr };

#ifdef DFMEXT_INTERFACE_Window
// 窗口的扩展
static DFMEXT::DFMExtWindowPlugin *myWindow { nullptr };
#endif

extern "C" void dfm_extension_initiliaze()
{
    myMenu = new Exapmle::MyMenuPlugin;
    myEmblemIcon = new Exapmle::MyEmblemIconPlugin;
#ifdef DFMEXT_INTERFACE_Window
    myWindow = new Exapmle::MyWindowPlugin;
#endif
}

extern "C" void dfm_extension_shutdown()
{
    delete myMenu;
    delete myEmblemIcon;
#ifdef DFMEXT_INTERFACE_Window
    delete myWindow;
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
