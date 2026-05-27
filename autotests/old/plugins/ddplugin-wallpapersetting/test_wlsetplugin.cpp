// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "wlsetplugin.h"
#include "wallpapersettings.h"
#include "private/autoactivatewindow.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QWidget>
#include <dfm-base/utils/sysinfoutils.h>

using namespace ddplugin_wallpapersetting;
using namespace dpf;

class UT_WlSetPlugin : public testing::Test {
protected:
    void SetUp() override {
        // DBus isolation: avoid touching real session bus
        stub.set_lamda(&QDBusConnection::sessionBus, []() -> QDBusConnection { __DBG_STUB_INVOKE__ return QDBusConnection(QStringLiteral("stub")); });

        using ConnectFn = bool (QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
        stub.set_lamda(static_cast<ConnectFn>(&QDBusConnection::connect), [](QDBusConnection *, const QString &, const QString &, const QString &, const QString &, const QString &, QObject *, const char *) { __DBG_STUB_INVOKE__ return false; });

        using RegObj1 = bool (QDBusConnection::*)(const QString &, const QString &, QObject *, QDBusConnection::RegisterOptions);
        using RegObj2 = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);
        using UnregObj = void (QDBusConnection::*)(const QString &, QDBusConnection::UnregisterMode);
        using RegSvc = bool (QDBusConnection::*)(const QString &);
        using CallFn = QDBusMessage (QDBusConnection::*)(const QDBusMessage &, QDBus::CallMode, int) const;
        using AsyncCallFn = QDBusPendingCall (QDBusConnection::*)(const QDBusMessage &, int) const;

        stub.set_lamda(static_cast<RegObj1>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, const QString &, QObject *, QDBusConnection::RegisterOptions) { __DBG_STUB_INVOKE__ return true; });
        stub.set_lamda(static_cast<RegObj2>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) { __DBG_STUB_INVOKE__ return true; });
        stub.set_lamda(static_cast<UnregObj>(&QDBusConnection::unregisterObject), [](QDBusConnection *, const QString &, QDBusConnection::UnregisterMode) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<RegSvc>(&QDBusConnection::registerService), [](QDBusConnection *, const QString &) { __DBG_STUB_INVOKE__ return true; });
        stub.set_lamda(static_cast<CallFn>(&QDBusConnection::call), [](const QDBusConnection *, const QDBusMessage &, QDBus::CallMode, int) -> QDBusMessage { __DBG_STUB_INVOKE__ return QDBusMessage(); });
        stub.set_lamda(static_cast<AsyncCallFn>(&QDBusConnection::asyncCall), [](const QDBusConnection *, const QDBusMessage &, int) -> QDBusPendingCall { __DBG_STUB_INVOKE__ return QDBusPendingCall::fromCompletedCall(QDBusMessage()); });
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

// [start]_[InitAndRegisterDBus]_[ReturnTrue]
TEST_F(UT_WlSetPlugin, Start_InitAndRegisterDBus_ReturnTrue)
{
    WlSetPlugin plugin;

    // Stub QDBusConnection::registerObject used in registerDBus
    using RegisterObj = bool (QDBusConnection::*)(const QString &, const QString &, QObject *, QDBusConnection::RegisterOptions);
    auto reg = static_cast<RegisterObj>(&QDBusConnection::registerObject);
    stub.set_lamda(reg, [&](QDBusConnection *, const QString &path, const QString &iface, QObject *, QDBusConnection::RegisterOptions) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(path, QString("/org/deepin/dde/desktop/wallpapersettings"));
        EXPECT_EQ(iface, QString("org.deepin.dde.desktop.wallpapersettings"));
        return true;
    });

    EXPECT_NO_THROW(plugin.initialize());
    EXPECT_TRUE(plugin.start());
    EXPECT_NO_THROW(plugin.stop());
}

// [EventHandle]_[wallpaperSettingOnDeepin23]_[ShowInternal]
TEST_F(UT_WlSetPlugin, EventHandle_wallpaperSettingOnDeepin23_ShowInternal)
{
    EventHandle handle;

    // Force Deepin23 branch
    stub.set_lamda(&DFMBASE_NAMESPACE::SysInfoUtils::isDeepin23, []() -> bool { __DBG_STUB_INVOKE__ return true; });

    // Observe EventHandle::show being invoked with expected parameters
    bool showCalled = false;
    QString gotName;
    int gotMode = -1;
    stub.set_lamda(ADDR(EventHandle, show), [&](EventHandle *, QString name, int mode) {
        __DBG_STUB_INVOKE__
        showCalled = true;
        gotName = name;
        gotMode = mode;
    });

    // Also stub AutoActivateWindow::start to avoid X11/Wayland interaction
    stub.set_lamda(ADDR(ddplugin_wallpapersetting::AutoActivateWindow, start), [](ddplugin_wallpapersetting::AutoActivateWindow *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(handle.wallpaperSetting("HDMI-1"));
    EXPECT_TRUE(showCalled);
    EXPECT_EQ(gotMode, static_cast<int>(WallpaperSettings::Mode::WallpaperMode));
}

// [EventHandle]_[screenSaverSetting]_[ShowScreenSaverMode]
TEST_F(UT_WlSetPlugin, EventHandle_screenSaverSetting_ShowScreenSaverMode)
{
    EventHandle handle;
    bool showCalled = false;
    int gotMode = -1;
    stub.set_lamda(ADDR(EventHandle, show), [&](EventHandle *, QString, int mode) {
        __DBG_STUB_INVOKE__ showCalled = true; gotMode = mode; });
    EXPECT_TRUE(handle.screenSaverSetting("HDMI-1"));
    EXPECT_TRUE(showCalled);
    EXPECT_EQ(gotMode, static_cast<int>(WallpaperSettings::Mode::ScreenSaverMode));
}

// [EventHandle]_[wallpaperSettingOnNonDeepin23]_[NoShowCalled]
TEST_F(UT_WlSetPlugin, EventHandle_wallpaperSettingOnNonDeepin23_NoShowCalled)
{
    EventHandle handle;

    // Force non-Deepin23 branch
    stub.set_lamda(&DFMBASE_NAMESPACE::SysInfoUtils::isDeepin23, []() -> bool { __DBG_STUB_INVOKE__ return false; });
    // Ensure EventHandle::show is NOT called in this branch
    bool showCalled = false;
    stub.set_lamda(ADDR(EventHandle, show), [&](EventHandle *, QString, int) { __DBG_STUB_INVOKE__ showCalled = true; });
    EXPECT_TRUE(handle.wallpaperSetting("HDMI-1"));
    EXPECT_FALSE(showCalled);
}

// [EventHandle]_[hookCanvasRequest]_[ForwardToWallpaperSetting]
TEST_F(UT_WlSetPlugin, EventHandle_hookCanvasRequest_ForwardToWallpaperSetting)
{
    EventHandle handle;
    bool called = false;
    using SetFn = bool (EventHandle::*)(const QString &);
    stub.set_lamda(static_cast<SetFn>(&EventHandle::wallpaperSetting), [&](EventHandle *, const QString &screen) {
        __DBG_STUB_INVOKE__
        called = (screen == "eDP-1");
        return true;
    });

    EXPECT_TRUE(handle.hookCanvasRequest("eDP-1"));
    EXPECT_TRUE(called);
}
