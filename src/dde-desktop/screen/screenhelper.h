// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
