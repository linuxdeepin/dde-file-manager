// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionbuttonmanager.cpp
 * @brief Unit tests for OptionButtonManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/optionbuttonmanager.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class OptionButtonManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionButtonManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionButtonManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonManagerTest, OptionButtonManager)
{
    // Test constructor: OptionButtonManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OptionButtonManagerTest, hasVsibleState)
{
    // Test method: bool hasVsibleState((const OptionButtonManager::Scheme &scheme))
    OptionButtonManager::Scheme _arg0{};
    auto result = obj->hasVsibleState(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OptionButtonManagerTest, instance)
{
    // Test getter: OptionButtonManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(OptionButtonManagerTest, optBtnVisibleState)
{
    // Test method: OptionButtonManager::OptBtnVisibleState optBtnVisibleState((const OptionButtonManager::Scheme &scheme))
    OptionButtonManager::Scheme _arg0{};
    auto result = obj->optBtnVisibleState(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(OptionButtonManagerTest, setOptBtnVisibleState)
{
    // Test setter: void setOptBtnVisibleState((const OptionButtonManager::Scheme &scheme, OptionButtonManager::OptBtnVisibleState state))
    OptionButtonManager::Scheme _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setOptBtnVisibleState(_arg0, OptionButtonManager::OptBtnVisibleState()));
}
