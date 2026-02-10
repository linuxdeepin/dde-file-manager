// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/shred/vitrualshredplugin.h"
#include "plugins/common/dfmplugin-utils/shred/shredutils.h"

#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_VirtualShredPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(ShredUtils, initDconfig),
                       [](ShredUtils *) {
                           __DBG_STUB_INVOKE__
                       });

        stub.set_lamda(ADDR(SettingJsonGenerator, addGroup),
                       [](SettingJsonGenerator *, const QString &, const QString &) {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        stub.set_lamda(ADDR(SettingJsonGenerator, addConfig),
                       [](SettingJsonGenerator *, const QString &, const QVariantMap &) {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        stub.set_lamda(ADDR(DialogManager, registerSettingWidget),
                       [](DialogManager *, const QString &, std::function<QWidget *(QObject *)>) {
                           __DBG_STUB_INVOKE__
                       });

        plugin = new VirtualShredPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualShredPlugin *plugin { nullptr };
};

TEST_F(UT_VirtualShredPlugin, Constructor_CreatesPlugin)
{
    EXPECT_NE(plugin, nullptr);
}

TEST_F(UT_VirtualShredPlugin, initialize_DoesNothing)
{
    plugin->initialize();
}

TEST_F(UT_VirtualShredPlugin, start_AllPluginsStarted_ReturnsTrue)
{
    stub.set_lamda(&LifeCycle::isAllPluginsStarted,
                   []() -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = plugin->start();

    EXPECT_TRUE(result);
}

TEST_F(UT_VirtualShredPlugin, start_PluginsNotStarted_ConnectsSignal)
{
    stub.set_lamda(&LifeCycle::isAllPluginsStarted,
                   []() -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = plugin->start();

    EXPECT_TRUE(result);
}
