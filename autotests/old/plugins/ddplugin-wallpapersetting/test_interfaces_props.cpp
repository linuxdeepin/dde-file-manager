// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "dbus/appearance_interface.h"
#include "dbus/screensaver_interface.h"

#include <QDBusConnection>
#include <QDBusPendingCall>

#include "stubext.h"

// Fixture to isolate DBus and async calls
class UT_InterfacesProps_Fixture : public ::testing::Test {
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        // Stub sessionBus and async calls to avoid real DBus
        stub.set_lamda(&QDBusConnection::sessionBus, []() -> QDBusConnection {
            __DBG_STUB_INVOKE__
            return QDBusConnection(QStringLiteral("stub"));
        });

        using AsyncCall = QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &);
        stub.set_lamda(static_cast<AsyncCall>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                       [](QDBusAbstractInterface *, const QString &, const QList<QVariant> &) -> QDBusPendingCall {
                           __DBG_STUB_INVOKE__
                           return QDBusPendingCall::fromCompletedCall(QDBusMessage());
                       });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// [Appearance_Interface]_[StaticAndPropertyGetter]_[NoCrash]
TEST_F(UT_InterfacesProps_Fixture, Appearance_StaticAndPropertyGetter_NoCrash)
{
    EXPECT_STREQ(Appearance_Interface::staticInterfaceName(), "org.deepin.dde.Appearance1");

    Appearance_Interface inter("org.deepin.dde.Appearance1",
                               "/org/deepin/dde/Appearance1",
                               QDBusConnection::sessionBus());
    // Access inline property getters (default QVariant in test env)
    (void)inter.background();
    (void)inter.cursorTheme();
    (void)inter.fontSize();
    (void)inter.globalTheme();
    (void)inter.gtkTheme();
    (void)inter.iconTheme();
    (void)inter.monospaceFont();
    (void)inter.opacity();
    (void)inter.qtActiveColor();
    (void)inter.standardFont();
    (void)inter.wallpaperSlideShow();
    (void)inter.wallpaperURls();
    (void)inter.windowRadius();
    // Cover setters
    inter.setFontSize(12.3);
    inter.setOpacity(0.8);
    inter.setQtActiveColor(QStringLiteral("#000000"));
    inter.setWallpaperSlideShow(QStringLiteral("off"));
    inter.setWindowRadius(10);
    SUCCEED();
}

// [Appearance_Interface]_[AllSlots]_[NoCrash]
TEST_F(UT_InterfacesProps_Fixture, Appearance_AllSlots_NoCrash)
{
    Appearance_Interface inter("org.deepin.dde.Appearance1",
                               "/org/deepin/dde/Appearance1",
                               QDBusConnection::sessionBus());
    // Call all DBus methods (async stubbed)
    (void)inter.Delete("theme", "name");
    (void)inter.GetCurrentWorkspaceBackground();
    (void)inter.GetCurrentWorkspaceBackgroundForMonitor("eDP-1");
    (void)inter.GetScaleFactor();
    (void)inter.GetScreenScaleFactors();
    (void)inter.GetWallpaperSlideShow("eDP-1");
    (void)inter.GetWorkspaceBackgroundForMonitor(0, "eDP-1");
    (void)inter.List("themes");
    (void)inter.Reset();
    (void)inter.Set("Opacity", "0.9");
    (void)inter.SetCurrentWorkspaceBackground("file:///wallpaper.jpg");
    (void)inter.SetCurrentWorkspaceBackgroundForMonitor("file:///wallpaper.jpg", "eDP-1");
    (void)inter.SetMonitorBackground("eDP-1", "file:///wallpaper.jpg");
    (void)inter.SetScaleFactor(1.25);
    ScaleFactors sf; sf.insert("eDP-1", 1.25);
    (void)inter.SetScreenScaleFactors(sf);
    (void)inter.SetWallpaperSlideShow("eDP-1", "off");
    (void)inter.SetWorkspaceBackgroundForMonitor(0, "eDP-1", "file:///wallpaper.jpg");
    (void)inter.Show("wallpaper", {"a", "b"});
    (void)inter.Thumbnail("wallpaper", "a");
    SUCCEED();
}

// [ComDeepinScreenSaverInterface]_[StaticAndPropertyGetter]_[NoCrash]
TEST_F(UT_InterfacesProps_Fixture, ScreenSaver_StaticAndPropertyGetter_NoCrash)
{
    EXPECT_STREQ(ComDeepinScreenSaverInterface::staticInterfaceName(), "com.deepin.ScreenSaver");
    ComDeepinScreenSaverInterface inter("com.deepin.ScreenSaver",
                                        "/com/deepin/ScreenSaver",
                                        QDBusConnection::sessionBus());
    (void)inter.allScreenSaver();
    (void)inter.batteryScreenSaverTimeout();
    (void)inter.currentScreenSaver();
    (void)inter.isRunning();
    (void)inter.linePowerScreenSaverTimeout();
    (void)inter.lockScreenAtAwake();
    (void)inter.lockScreenDelay();
    SUCCEED();
}
