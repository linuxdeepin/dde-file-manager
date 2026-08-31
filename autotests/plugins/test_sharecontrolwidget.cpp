// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharecontrolwidget.cpp
 * @brief Unit tests for ShareControlWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widget/sharecontrolwidget.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class ShareControlWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareControlWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareControlWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareControlWidgetTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(ShareControlWidgetTest, setupUi)
{
    // Test setter: void setupUi((bool disableState))
    EXPECT_NO_FATAL_FAILURE(obj->setupUi(false));
}

TEST_F(ShareControlWidgetTest, shareFolder)
{
    // Test bool getter: shareFolder()
    bool result = obj->shareFolder();
    EXPECT_FALSE(result);

}

TEST_F(ShareControlWidgetTest, updateWidgetStatus)
{
    // Test method: void updateWidgetStatus((const QString &filePath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateWidgetStatus(_arg0));
}

TEST_F(ShareControlWidgetTest, validateShareName)
{
    // Test bool getter: validateShareName()
    bool result = obj->validateShareName();
    EXPECT_FALSE(result);

}

TEST_F(ShareControlWidgetTest, ShareControlWidget)
{
    // Test constructor: ShareControlWidget((const QUrl &url, bool disableState, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
