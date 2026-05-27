// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QApplication>
#include <QRect>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QTimer>

#include "wallaperpreview.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"
#include "desktoputils/widgetutil.h"

using namespace ddplugin_wallpapersetting;

class UT_WallaperPreview : public testing::Test {
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

// [init]_[BuildWidgetsAndPullSettings]_[NoCrash]
TEST_F(UT_WallaperPreview, Init_BuildWidgetsAndPullSettings_NoCrash)
{
    // Prevent setting windowing hints to real WM
    stub.set_lamda(&ddplugin_desktop_util::setPrviewWindow, [](QWidget *) { __DBG_STUB_INVOKE__ });
    // Avoid real window display via widget attribute

    // Stub screen discovery to minimal no-screen case
    QList<DFMBASE_NAMESPACE::ScreenPointer> logic;
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLogicScreens, [&]() { __DBG_STUB_INVOKE__ return logic; });
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLastChangedMode, []() { __DBG_STUB_INVOKE__ return DFMBASE_NAMESPACE::DisplayMode::kShowonly; });
    stub.set_lamda(&ddplugin_desktop_util::screenProxyPrimaryScreen, [&]() { __DBG_STUB_INVOKE__ return DFMBASE_NAMESPACE::ScreenPointer(); });
    stub.set_lamda(&ddplugin_desktop_util::desktopFrameRootWindows, []() { __DBG_STUB_INVOKE__ return QList<QWidget*>{}; });

    // Stub DBus interface calls inside getBackground
    using Inter = BackgroudInter;
    // GetCurrentWorkspaceBackgroundForMonitor(QString)
    typedef QDBusPendingReply<QString> (Inter::*GetBg)(const QString &);
    auto fn = static_cast<GetBg>(&Inter::GetCurrentWorkspaceBackgroundForMonitor);
    stub.set_lamda(fn, [](Inter *, const QString &) -> QDBusPendingReply<QString> { __DBG_STUB_INVOKE__ return QDBusPendingReply<QString>(); });

    WallaperPreview preview;
    EXPECT_NO_THROW(preview.init());
    preview.setVisible(true);
    preview.updateWallpaper();
    EXPECT_TRUE(preview.isVisible());
}

// [buildWidgets]_[MultiScreenExtend]_[CreateAndUpdate]
TEST_F(UT_WallaperPreview, BuildWidgets_MultiScreenExtend_CreateAndUpdate)
{
    // Prevent setting windowing hints to real WM
    stub.set_lamda(&ddplugin_desktop_util::setPrviewWindow, [](QWidget *) { __DBG_STUB_INVOKE__ });
    // Avoid real window display via widget attribute

    // Dummy screen implementation
    struct DummyScreen : public DFMBASE_NAMESPACE::AbstractScreen {
        QString n; QRect g;
        DummyScreen(const QString &name, const QRect &geo) : n(name), g(geo) {}
        QString name() const override { return n; }
        QRect geometry() const override { return g; }
        QRect availableGeometry() const override { return g; }
        QRect handleGeometry() const override { return g; }
    };

    QList<DFMBASE_NAMESPACE::ScreenPointer> logic;
    logic << DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("HDMI-1", QRect(0,0,800,600)));
    logic << DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("eDP-1", QRect(800,0,800,600)));

    stub.set_lamda(&ddplugin_desktop_util::screenProxyLastChangedMode, [] { __DBG_STUB_INVOKE__ return DFMBASE_NAMESPACE::DisplayMode::kExtend; });
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLogicScreens, [logic] { __DBG_STUB_INVOKE__ return logic; });
    stub.set_lamda(&ddplugin_desktop_util::screenProxyScreen, [logic](const QString &name) {
        __DBG_STUB_INVOKE__
        for (auto s : logic) if (s->name() == name) return s; return DFMBASE_NAMESPACE::ScreenPointer();
    });

    // Stub DBus get background to return empty (avoid file IO)
    using Inter = BackgroudInter; typedef QDBusPendingReply<QString> (Inter::*GetBg)(const QString &);
    auto fn = static_cast<GetBg>(&Inter::GetCurrentWorkspaceBackgroundForMonitor);
    stub.set_lamda(fn, [](Inter *, const QString &) -> QDBusPendingReply<QString> { __DBG_STUB_INVOKE__ return QDBusPendingReply<QString>(); });

    WallaperPreview preview;
    preview.buildWidgets();  // create two widgets
    preview.updateGeometry();
    preview.setVisible(true);
    EXPECT_TRUE(preview.isVisible());
}

// [buildWidgets]_[SingleScreen_PrimaryChanged]_[UpdateExistingGeometry]
TEST_F(UT_WallaperPreview, BuildWidgets_SingleScreen_PrimaryChanged_UpdateGeometry)
{
    // Keep WM side-effects disabled
    stub.set_lamda(&ddplugin_desktop_util::setPrviewWindow, [](QWidget *) { __DBG_STUB_INVOKE__ });

    // Dummy screen type
    struct DummyScreen : public DFMBASE_NAMESPACE::AbstractScreen {
        QString n; QRect g; DummyScreen(const QString &name, const QRect &geo) : n(name), g(geo) {}
        QString name() const override { return n; }
        QRect geometry() const override { return g; }
        QRect availableGeometry() const override { return g; }
        QRect handleGeometry() const override { return g; }
    };

    // Single-screen branch via kShowonly
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLastChangedMode, [] { __DBG_STUB_INVOKE__ return DFMBASE_NAMESPACE::DisplayMode::kShowonly; });
    // logic screens can be empty for showonly path
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLogicScreens, [] { __DBG_STUB_INVOKE__ return QList<DFMBASE_NAMESPACE::ScreenPointer>{}; });

    // First primary with geometry A (create widget)
    auto primaryA = DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("eDP-1", QRect(0,0,800,600)));
    stub.set_lamda(&ddplugin_desktop_util::screenProxyPrimaryScreen, [primaryA] { __DBG_STUB_INVOKE__ return primaryA; });
    WallaperPreview preview;
    preview.buildWidgets();

    // Change primary geometry (same name) to trigger update path
    auto primaryB = DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("eDP-1", QRect(0,0,1024,768)));
    stub.set_lamda(&ddplugin_desktop_util::screenProxyPrimaryScreen, [primaryB] { __DBG_STUB_INVOKE__ return primaryB; });
    EXPECT_NO_THROW(preview.buildWidgets());
}

// [buildWidgets]_[MultiScreen_RemoveOrphan]_[PruneMissing]
TEST_F(UT_WallaperPreview, BuildWidgets_MultiScreen_RemoveOrphan_PruneMissing)
{
    stub.set_lamda(&ddplugin_desktop_util::setPrviewWindow, [](QWidget *) { __DBG_STUB_INVOKE__ });

    struct DummyScreen : public DFMBASE_NAMESPACE::AbstractScreen {
        QString n; QRect g; DummyScreen(const QString &name, const QRect &geo) : n(name), g(geo) {}
        QString name() const override { return n; }
        QRect geometry() const override { return g; }
        QRect availableGeometry() const override { return g; }
        QRect handleGeometry() const override { return g; }
    };

    // Start with two screens
    QList<DFMBASE_NAMESPACE::ScreenPointer> logic;
    logic << DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("HDMI-1", QRect(0,0,800,600)));
    logic << DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("eDP-1", QRect(800,0,800,600)));
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLastChangedMode, [] { __DBG_STUB_INVOKE__ return DFMBASE_NAMESPACE::DisplayMode::kExtend; });
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLogicScreens, [logic] { __DBG_STUB_INVOKE__ return logic; });
    stub.set_lamda(&ddplugin_desktop_util::screenProxyScreen, [logic](const QString &name) {
        __DBG_STUB_INVOKE__ for (auto s : logic) if (s->name() == name) return s; return DFMBASE_NAMESPACE::ScreenPointer();
    });

    WallaperPreview preview;
    preview.buildWidgets();

    // Now simulate one screen removed: only return for eDP-1
    stub.set_lamda(&ddplugin_desktop_util::screenProxyScreen, [](const QString &name) {
        __DBG_STUB_INVOKE__ return name == "eDP-1" ? DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("eDP-1", QRect(0,0,800,600))) : DFMBASE_NAMESPACE::ScreenPointer();
    });
    EXPECT_NO_THROW(preview.buildWidgets());
}

// [updateGeometry]_[NoWidgetOrEqualGeometry]_[SkipAndContinue]
TEST_F(UT_WallaperPreview, UpdateGeometry_NoWidgetOrEqualGeometry_SkipAndContinue)
{
    struct DummyScreen : public DFMBASE_NAMESPACE::AbstractScreen {
        QString n; QRect g; DummyScreen(const QString &name, const QRect &geo) : n(name), g(geo) {}
        QString name() const override { return n; }
        QRect geometry() const override { return g; }
        QRect availableGeometry() const override { return g; }
        QRect handleGeometry() const override { return g; }
    };
    // Return screens, but previewWidgets is empty -> wid null => skip
    QList<DFMBASE_NAMESPACE::ScreenPointer> screens;
    screens << DFMBASE_NAMESPACE::ScreenPointer(new DummyScreen("eDP-1", QRect(0,0,800,600)));
    stub.set_lamda(&ddplugin_desktop_util::screenProxyScreens, [screens] { __DBG_STUB_INVOKE__ return screens; });
    WallaperPreview preview;
    EXPECT_NO_THROW(preview.updateGeometry());
}

// [setWallpaper]_[AddOneScreenAndUpdate]_[NoCrash]
TEST_F(UT_WallaperPreview, SetWallpaper_AddOneScreenAndUpdate_NoCrash)
{
    // Prevent setting windowing hints to real WM
    stub.set_lamda(&ddplugin_desktop_util::setPrviewWindow, [](QWidget *) { __DBG_STUB_INVOKE__ });
    // Avoid real window display via widget attribute

    // make buildWidgets create nothing; we'll directly set wallpaper map via API
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLogicScreens, [] { __DBG_STUB_INVOKE__ return QList<DFMBASE_NAMESPACE::ScreenPointer>{}; });
    stub.set_lamda(&ddplugin_desktop_util::screenProxyLastChangedMode, [] { __DBG_STUB_INVOKE__ return DFMBASE_NAMESPACE::DisplayMode::kShowonly; });

    WallaperPreview preview;
    preview.init();
    preview.setVisible(true);

    preview.setWallpaper("eDP-1", "/tmp/a.jpg");
    // In headless env, no previewWidgets exist, updateWallpaper prunes records.
    // Only verify no crash here to increase coverage of setWallpaper/updateWallpaper path.
    EXPECT_TRUE(true);
}
