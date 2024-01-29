#include <dfm-extension/dfm-extension.h>

#include "mymenuplugin.h"
#include "myemblemiconplugin.h"
#include "mywindowplugin.h"

#ifdef DFMEXT_VERSION
#    if (DFMEXT_VERSION >= DFMEXT_VERSION_CHECK(6, 0, 0))
#        define NEW_VERSION
#    endif
#endif

// 右键菜单的扩展
static DFMEXT::DFMExtMenuPlugin *myMenu { nullptr };
// 角标的扩展
static DFMEXT::DFMExtEmblemIconPlugin *myEmblemIcon { nullptr };

#ifdef NEW_VERSION
// 窗口的扩展
static DFMEXT::DFMExtWindowPlugin *myWindow { nullptr };
#endif

extern "C" void dfm_extension_initiliaze()
{
    myMenu = new Exapmle::MyMenuPlugin;
    myEmblemIcon = new Exapmle::MyEmblemIconPlugin;
#ifdef NEW_VERSION
    myWindow = new Exapmle::MyWindowPlugin;
#endif
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

#ifdef NEW_VERSION
extern "C" DFMEXT::DFMExtWindowPlugin *dfm_extension_window()
{
    return myWindow;
}
#endif
