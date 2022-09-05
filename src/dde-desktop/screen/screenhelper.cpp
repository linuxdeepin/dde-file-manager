// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenhelper.h"
#include "util/dde/desktopinfo.h"
#include "screenmanager.h"
#include "screenmanagerwayland.h"

AbstractScreenManager *ScreenHelper::screenManager()
{
    if (DesktopInfo().waylandDectected()){
        static ScreenManagerWayland ins;
        return &ins;
    }
    else {
        static ScreenManager ins;
        return &ins;
    }
}

ScreenHelper::ScreenHelper()
{

}

