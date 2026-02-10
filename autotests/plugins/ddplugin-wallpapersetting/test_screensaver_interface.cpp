// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "dbus/screensaver_interface.h"
#include <QDBusConnection>

// [ComDeepinScreenSaverInterface]_[ConstructDestruct]_[NoCrash]
TEST(UT_ScreenSaverInterface_Original, ConstructDestruct_NoCrash)
{
    ComDeepinScreenSaverInterface inter("com.deepin.ScreenSaver",
                                        "/com/deepin/ScreenSaver",
                                        QDBusConnection::sessionBus());
    EXPECT_STREQ(inter.metaObject()->className(), "ComDeepinScreenSaverInterface");
}
