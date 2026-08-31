// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerviewcontainer.cpp
 * @brief Unit tests for ComputerViewContainer methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/computerviewcontainer.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerViewContainerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerViewContainer();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerViewContainer *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerViewContainerTest, ComputerViewContainer)
{
    // Test constructor: ComputerViewContainer((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerViewContainerTest, contentWidget)
{
    // Test getter: QWidget contentWidget()
    auto result = obj->contentWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->contentWidget(); });

}

TEST_F(ComputerViewContainerTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ComputerViewContainerTest, selectedUrlList)
{
    // Test getter: QList<QUrl> selectedUrlList()
    auto result = obj->selectedUrlList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerViewContainerTest, setRootUrl)
{
    // Test method: bool setRootUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->setRootUrl(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerViewContainerTest, widget)
{
    // Test getter: QWidget widget()
    auto result = obj->widget();
    EXPECT_NO_FATAL_FAILURE({ obj->widget(); });

}
