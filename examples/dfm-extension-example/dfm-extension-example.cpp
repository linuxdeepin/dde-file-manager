#include <dfm-extension/dfm-extension.h>

#include "mymenuplugin.h"
#include "myemblemiconplugin.h"

// 右键菜单的扩展
static DFMEXT::DFMExtMenuPlugin *myMenu { nullptr };
// 角标的扩展
static DFMEXT::DFMExtEmblemIconPlugin *myEmblemIcon { nullptr };

extern "C" void dfm_extension_initiliaze()
{
    myMenu = new Exapmle::MyMenuPlugin;
    myEmblemIcon = new Exapmle::MyEmblemIconPlugin;
}

extern "C" void dfm_extension_shutdown()
{
    delete myMenu;
    delete myEmblemIcon;
}

extern "C" DFMEXT::DFMExtMenuPlugin *dfm_extension_menu()
{
    return myMenu;
}

extern "C" DFMEXT::DFMExtEmblemIconPlugin *dfm_extension_emblem()
{
    return myEmblemIcon;
}
