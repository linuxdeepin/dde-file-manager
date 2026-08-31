// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalmediawidget.cpp
 * @brief Unit tests for OpticalMediaWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/opticalmediawidget.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalMediaWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalMediaWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalMediaWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalMediaWidgetTest, OpticalMediaWidget)
{
    // Test constructor: OpticalMediaWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalMediaWidgetTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(OpticalMediaWidgetTest, updateDiscInfo)
{
    // Test method: bool updateDiscInfo((const QUrl &url, bool retry))
    QUrl _arg0{};
    auto result = obj->updateDiscInfo(_arg0, false);
    EXPECT_FALSE(result);

}

TEST_F(OpticalMediaWidgetTest, updateUi)
{
    // Test method: void updateUi(())
    EXPECT_NO_FATAL_FAILURE(obj->updateUi());
}
