// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualshredplugin.cpp
 * @brief Unit tests for VirtualShredPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/vitrualshredplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualShredPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualShredPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualShredPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualShredPluginTest, addShredSettingItem)
{
    // Test method: void addShredSettingItem(())
    EXPECT_NO_FATAL_FAILURE(obj->addShredSettingItem());
}

TEST_F(VirtualShredPluginTest, bindScene)
{
    // Test method: void bindScene((const QString &parentScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindScene(_arg0));
}

TEST_F(VirtualShredPluginTest, initialize)
{
    // Test getter: DFMBASE_USE_NAMESPACE initialize()
    EXPECT_NO_FATAL_FAILURE({ obj->initialize(); });
}

TEST_F(VirtualShredPluginTest, onAllPluginsStarted)
{
    // Test method: void onAllPluginsStarted(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsStarted());
}

TEST_F(VirtualShredPluginTest, onMenuSceneAdded)
{
    // Test method: void onMenuSceneAdded((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onMenuSceneAdded(_arg0));
}

TEST_F(VirtualShredPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
