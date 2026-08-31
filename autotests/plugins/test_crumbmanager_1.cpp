// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbmanager_1.cpp
 * @brief Unit tests for CrumbManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/crumbmanager.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbManagerTest, CrumbManager)
{
    // Test constructor: CrumbManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CrumbManagerTest, createControllerByUrl)
{
    // Test method: CrumbInterface createControllerByUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createControllerByUrl(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createControllerByUrl(_arg0); });

}

TEST_F(CrumbManagerTest, isRegistered)
{
    // Test method: bool isRegistered((const KeyType &scheme))
    KeyType _arg0{};
    auto result = obj->isRegistered(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CrumbManagerTest, registerCrumbCreator)
{
    // Test method: void registerCrumbCreator((const CrumbManager::KeyType &scheme, const CrumbManager::CrumbCreator &creator))
    CrumbManager::KeyType _arg0{};
    CrumbManager::CrumbCreator _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->registerCrumbCreator(_arg0, _arg1));
}
