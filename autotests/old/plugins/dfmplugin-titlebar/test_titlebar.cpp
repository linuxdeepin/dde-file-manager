// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "titlebar.h"
#include "utils/titlebarhelper.h"
#include "views/titlebarwidget.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class TitleBarPluginTest : public testing::Test
{
protected:
    void SetUp() override
    {
        plugin = new TitleBar();
        stub.clear();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    TitleBar *plugin { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TitleBarPluginTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(plugin, nullptr);
}

TEST_F(TitleBarPluginTest, Initialize_ConnectsWindowSignals_SignalsConnected)
{
    EXPECT_NO_THROW(plugin->initialize());
}

TEST_F(TitleBarPluginTest, Start_RegistersFileScheme_SchemeRegistered)
{
    bool schemeRegistered = false;

    stub.set_lamda(static_cast<QVariant (EventChannelManager::*)(const QString &, const QString &, QString, QVariantMap &&)>(&EventChannelManager::push),
                   [&schemeRegistered](DPF_NAMESPACE::EventChannelManager *, const QString &space, const QString &topic, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_titlebar" && topic == "slot_Custom_Register")
                           schemeRegistered = true;
                       return true;
                   });

    stub.set_lamda(&DPF_NAMESPACE::LifeCycle::pluginMetaObj, [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    typedef void (SettingBackend::*Func)(const QString &, SettingBackend::GetOptFunc, SettingBackend::SaveOptFunc);
    stub.set_lamda(static_cast<Func>(&SettingBackend::addSettingAccessor),
                   [] {
                       __DBG_STUB_INVOKE__
                   });

    plugin->start();
    EXPECT_TRUE(schemeRegistered);
}

TEST_F(TitleBarPluginTest, Start_SearchPluginStarted_SearchEnabled)
{
    // Stub pluginMetaObj to return a started plugin
    PluginMetaObjectPointer mockMeta(new DPF_NAMESPACE::PluginMetaObject());
    stub.set_lamda(&DPF_NAMESPACE::LifeCycle::pluginMetaObj, [mockMeta] {
        __DBG_STUB_INVOKE__
        return mockMeta;
    });
    stub.set_lamda(&DPF_NAMESPACE::PluginMetaObject::pluginState, [](DPF_NAMESPACE::PluginMetaObject *) {
        __DBG_STUB_INVOKE__
        return DPF_NAMESPACE::PluginMetaObject::kStarted;
    });

    stub.set_lamda(static_cast<QVariant (EventChannelManager::*)(const QString &, const QString &, QString, QVariantMap &&)>(&EventChannelManager::push),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    plugin->start();
    EXPECT_TRUE(TitleBarHelper::searchEnabled);
}

TEST_F(TitleBarPluginTest, OnWindowCreated_ValidWindowId_TitleBarWidgetCreated)
{
    quint64 windowId = 12345;
    bool titleBarAdded = false;

    stub.set_lamda(&TitleBarHelper::addTileBar, [&titleBarAdded](quint64, TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        titleBarAdded = true;
    });

    stub.set_lamda(static_cast<QVariant (EventChannelManager::*)(const QString &, const QString &, QWidget *, const char(&)[19])>(&EventChannelManager::push),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    plugin->onWindowCreated(windowId);
    EXPECT_TRUE(titleBarAdded);
}

TEST_F(TitleBarPluginTest, OnWindowOpened_ValidWindowId_TitleBarInstalled)
{
    quint64 windowId = 12345;
    bool titleBarInstalled = false;

    // Create mock objects
    auto mockWindow = new FileManagerWindow(QUrl());
    auto mockTitleBar = new TitleBarWidget();
    auto mockNavWidget = new NavWidget();

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [mockWindow](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return mockWindow;
    });

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [mockTitleBar](quint64) {
        __DBG_STUB_INVOKE__
        return mockTitleBar;
    });

    stub.set_lamda(&TitleBarHelper::createSettingsMenu, [](quint64) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&FileManagerWindow::installTitleBar, [&titleBarInstalled](FileManagerWindow *, QWidget *) {
        __DBG_STUB_INVOKE__
        titleBarInstalled = true;
    });

    stub.set_lamda(&TitleBarWidget::navWidget, [&mockNavWidget](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return mockNavWidget;
    });

    plugin->onWindowOpened(windowId);
    EXPECT_TRUE(titleBarInstalled);

    delete mockWindow;
    delete mockTitleBar;
    delete mockNavWidget;
}

TEST_F(TitleBarPluginTest, OnWindowClosed_ValidWindowId_TitleBarRemoved)
{
    quint64 windowId = 12345;
    bool titleBarRemoved = false;

    stub.set_lamda(&TitleBarHelper::removeTitleBar, [&titleBarRemoved](quint64) {
        __DBG_STUB_INVOKE__
        titleBarRemoved = true;
    });

    plugin->onWindowClosed(windowId);
    EXPECT_TRUE(titleBarRemoved);
}
