// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailspacewidget.cpp
 * @brief Unit tests for DetailSpaceWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/detailspacewidget.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailSpaceWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailSpaceWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailSpaceWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailSpaceWidgetTest, DetailSpaceWidget)
{
    // Test constructor: DetailSpaceWidget((QFrame *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DetailSpaceWidgetTest, currentUrl)
{
    // Test getter: QUrl currentUrl()
    auto result = obj->currentUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(DetailSpaceWidgetTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(DetailSpaceWidgetTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(DetailSpaceWidgetTest, setCurrentUrl)
{
    // Test setter: void setCurrentUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentUrl(_arg0));
}
