#include "screenhelper.h"
#include "util/dde/desktopinfo.h"
#include "screenmanager.h"

AbstractScreenManager *ScreenHelper::screenManager()
{
    if (DesktopInfo().waylandDectected()){
        return nullptr;
    }
    else {
        static ScreenManager ins;
        return &ins;
    }
}

ScreenHelper::ScreenHelper()
{

}

