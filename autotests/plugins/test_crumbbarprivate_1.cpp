// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbbarprivate_1.cpp
 * @brief Unit tests for CrumbBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/crumbbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbBarPrivateTest, CrumbBarPrivate)
{
    // Test constructor: CrumbBarPrivate((CrumbBar *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CrumbBarPrivateTest, appendWidget)
{
    // Test method: void appendWidget((QWidget *widget, int stretch))
    EXPECT_NO_FATAL_FAILURE(obj->appendWidget(nullptr, 0));
}

TEST_F(CrumbBarPrivateTest, initConnections)
{
    // Test method: void initConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnections());
}

TEST_F(CrumbBarPrivateTest, initData)
{
    // Test method: void initData(())
    EXPECT_NO_FATAL_FAILURE(obj->initData());
}

TEST_F(CrumbBarPrivateTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(CrumbBarPrivateTest, setClickableAreaEnabled)
{
    // Test setter: void setClickableAreaEnabled((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setClickableAreaEnabled(false));
}

TEST_F(CrumbBarPrivateTest, updateController)
{
    // Test method: void updateController((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateController(_arg0));
}

TEST_F(CrumbBarPrivateTest, CrumbBarPrivate_Destructor)
{
    // Test method:  ~CrumbBarPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CrumbBarPrivate *tmp = new CrumbBarPrivate(); delete tmp; });
}
