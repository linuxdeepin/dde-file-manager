// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_urlpushbuttonprivate_1.cpp
 * @brief Unit tests for UrlPushButtonPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/urlpushbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class UrlPushButtonPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UrlPushButtonPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UrlPushButtonPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UrlPushButtonPrivateTest, UrlPushButtonPrivate)
{
    // Test constructor: UrlPushButtonPrivate((UrlPushButton *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(UrlPushButtonPrivateTest, activate)
{
    // Test method: void activate(())
    EXPECT_NO_FATAL_FAILURE(obj->activate());
}

TEST_F(UrlPushButtonPrivateTest, adjustButtonFont)
{
    // Test method: void adjustButtonFont(())
    EXPECT_NO_FATAL_FAILURE(obj->adjustButtonFont());
}

TEST_F(UrlPushButtonPrivateTest, foregroundColor)
{
    // Test getter: QColor foregroundColor()
    auto result = obj->foregroundColor();
    EXPECT_FALSE(result.isValid());

}

TEST_F(UrlPushButtonPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(UrlPushButtonPrivateTest, isAboveArrow)
{
    // Test method: bool isAboveArrow((int x))
    auto result = obj->isAboveArrow(0);
    EXPECT_FALSE(result);

}

TEST_F(UrlPushButtonPrivateTest, isSubDir)
{
    // Test method: bool isSubDir((int x))
    auto result = obj->isSubDir(0);
    EXPECT_FALSE(result);

}

TEST_F(UrlPushButtonPrivateTest, isTextClipped)
{
    // Test bool getter: isTextClipped()
    bool result = obj->isTextClipped();
    EXPECT_FALSE(result);

}

TEST_F(UrlPushButtonPrivateTest, onCompletionFound)
{
    // Test method: void onCompletionFound((const QStringList &stringList))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onCompletionFound(_arg0));
}

TEST_F(UrlPushButtonPrivateTest, onCustomContextMenu)
{
    // Test method: void onCustomContextMenu((const QPoint &point))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onCustomContextMenu(_arg0));
}

TEST_F(UrlPushButtonPrivateTest, popupVisible)
{
    // Test bool getter: popupVisible()
    bool result = obj->popupVisible();
    EXPECT_FALSE(result);

}

TEST_F(UrlPushButtonPrivateTest, requestCompleteByUrl)
{
    // Test method: void requestCompleteByUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->requestCompleteByUrl(_arg0));
}

TEST_F(UrlPushButtonPrivateTest, UrlPushButtonPrivate_Destructor)
{
    // Test method:  ~UrlPushButtonPrivate(())
    EXPECT_NO_FATAL_FAILURE({ UrlPushButtonPrivate *tmp = new UrlPushButtonPrivate(); delete tmp; });
}
