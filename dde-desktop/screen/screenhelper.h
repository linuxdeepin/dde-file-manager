#ifndef SCREENHELPER_H
#define SCREENHELPER_H

#include "abstractscreenmanager.h"

#define ScreenMrg ScreenHelper::screenManager()
class ScreenHelper
{
public:
    static AbstractScreenManager *screenManager();
private:
    ScreenHelper();
};

#endif // SCREENHELPER_H
