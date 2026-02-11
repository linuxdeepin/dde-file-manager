// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include "settingsdbusinterface.h"
#include "wlsetplugin.h"

using namespace ddplugin_wallpapersetting;

class UT_SettingsDBusInterface : public testing::Test {
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

// [ShowWallpaperChooser]_[ForwardToEventHandle]_[EventInvoked]
TEST_F(UT_SettingsDBusInterface, ShowWallpaperChooser_ForwardToEventHandle_EventInvoked)
{
    EventHandle handle;
    SettingsDBusInterface ifs(&handle);

    bool called = false;
    stub.set_lamda(ADDR(EventHandle, wallpaperSetting), [&](EventHandle *, const QString &name) -> bool {
        __DBG_STUB_INVOKE__
        called = true;
        EXPECT_EQ(name, QString("HDMI-1"));
        return true;
    });

    ifs.ShowWallpaperChooser("HDMI-1");
    EXPECT_TRUE(called);
}

// [ShowScreensaverChooser]_[ForwardToEventHandle]_[EventInvoked]
TEST_F(UT_SettingsDBusInterface, ShowScreensaverChooser_ForwardToEventHandle_EventInvoked)
{
    EventHandle handle;
    SettingsDBusInterface ifs(&handle);

    bool called = false;
    stub.set_lamda(ADDR(EventHandle, screenSaverSetting), [&](EventHandle *, const QString &name) -> bool {
        __DBG_STUB_INVOKE__
        called = true;
        EXPECT_EQ(name, QString("eDP-1"));
        return true;
    });

    ifs.ShowScreensaverChooser("eDP-1");
    EXPECT_TRUE(called);
}
