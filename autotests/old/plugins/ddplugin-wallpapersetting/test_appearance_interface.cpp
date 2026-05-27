// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "dbus/appearance_interface.h"
#include <QDBusConnection>

// [Appearance_Interface]_[ConstructDestruct]_[NoCrash]
TEST(UT_AppearanceInterface, ConstructDestruct_NoCrash)
{
    Appearance_Interface inter("org.deepin.dde.Appearance1",
                               "/org/deepin/dde/Appearance1",
                               QDBusConnection::sessionBus());
    EXPECT_STREQ(inter.metaObject()->className(), "Appearance_Interface");
}
