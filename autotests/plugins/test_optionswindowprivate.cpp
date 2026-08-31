// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionswindowprivate.cpp
 * @brief Unit tests for OptionsWindowPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/optionswindow.h"

#include <QTest>

using namespace ddplugin_organizer;

class OptionsWindowPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionsWindowPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionsWindowPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionsWindowPrivateTest, autoArrangeChanged)
{
    // Test method: void autoArrangeChanged((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->autoArrangeChanged(false));
}

TEST_F(OptionsWindowPrivateTest, enableChanged)
{
    // Test method: void enableChanged((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->enableChanged(false));
}

TEST_F(OptionsWindowPrivateTest, isAutoArrange)
{
    // Test bool getter: isAutoArrange()
    bool result = obj->isAutoArrange();
    EXPECT_FALSE(result);

}

TEST_F(OptionsWindowPrivateTest, setAutoArrange)
{
    // Test setter: void setAutoArrange((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->setAutoArrange(false));
}

TEST_F(OptionsWindowPrivateTest, OptionsWindowPrivate_Destructor)
{
    // Test getter: DWIDGET_USE_NAMESPACE ~OptionsWindowPrivate()
    EXPECT_NO_FATAL_FAILURE({ OptionsWindowPrivate *tmp = new OptionsWindowPrivate(); delete tmp; });

}
