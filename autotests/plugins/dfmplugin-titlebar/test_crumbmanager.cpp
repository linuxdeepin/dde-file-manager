// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/crumbmanager.h"
#include "utils/crumbinterface.h"

#include <gtest/gtest.h>
#include <QUrl>

using namespace dfmplugin_titlebar;

class CrumbManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(CrumbManagerTest, Instance_Singleton_ReturnsSameInstance)
{
    auto manager1 = CrumbManager::instance();
    auto manager2 = CrumbManager::instance();
    EXPECT_EQ(manager1, manager2);
    EXPECT_NE(manager1, nullptr);
}

TEST_F(CrumbManagerTest, RegisterCrumbCreator_ValidScheme_CreatorRegistered)
{
    auto creator = []() { return new CrumbInterface(); };
    CrumbManager::instance()->registerCrumbCreator("test", creator);
    EXPECT_TRUE(CrumbManager::instance()->isRegistered("test"));
}

TEST_F(CrumbManagerTest, IsRegistered_UnregisteredScheme_ReturnsFalse)
{
    EXPECT_FALSE(CrumbManager::instance()->isRegistered("unregistered_scheme_xyz"));
}

TEST_F(CrumbManagerTest, IsRegistered_RegisteredScheme_ReturnsTrue)
{
    auto creator = []() { return new CrumbInterface(); };
    CrumbManager::instance()->registerCrumbCreator("registered", creator);
    EXPECT_TRUE(CrumbManager::instance()->isRegistered("registered"));
}

TEST_F(CrumbManagerTest, CreateControllerByUrl_RegisteredScheme_ReturnsController)
{
    auto creator = []() { return new CrumbInterface(); };
    CrumbManager::instance()->registerCrumbCreator("myscheme", creator);

    QUrl url("myscheme://path/to/resource");
    CrumbInterface *controller = CrumbManager::instance()->createControllerByUrl(url);
    EXPECT_NE(controller, nullptr);
    delete controller;
}

TEST_F(CrumbManagerTest, CreateControllerByUrl_UnregisteredScheme_ReturnsNull)
{
    QUrl url("unregistered://path");
    CrumbInterface *controller = CrumbManager::instance()->createControllerByUrl(url);
    EXPECT_EQ(controller, nullptr);
}

TEST_F(CrumbManagerTest, RegisterCrumbCreator_MultipleSchemes_AllRegistered)
{
    auto creator1 = []() { return new CrumbInterface(); };
    auto creator2 = []() { return new CrumbInterface(); };

    CrumbManager::instance()->registerCrumbCreator("scheme1", creator1);
    CrumbManager::instance()->registerCrumbCreator("scheme2", creator2);

    EXPECT_TRUE(CrumbManager::instance()->isRegistered("scheme1"));
    EXPECT_TRUE(CrumbManager::instance()->isRegistered("scheme2"));
}
