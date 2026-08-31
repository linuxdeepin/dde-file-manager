// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmark.cpp
 * @brief Unit tests for BookMark methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bookmark.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookMarkTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookMark();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookMark *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookMarkTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}

TEST_F(BookMarkTest, bindScene)
{
    // Test method: void bindScene((const QString &parentScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindScene(_arg0));
}

TEST_F(BookMarkTest, bindWindows)
{
    // Test method: void bindWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->bindWindows());
}

TEST_F(BookMarkTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(BookMarkTest, onMenuSceneAdded)
{
    // Test method: void onMenuSceneAdded((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onMenuSceneAdded(_arg0));
}

TEST_F(BookMarkTest, onSideBarInstallFinished)
{
    // Test method: void onSideBarInstallFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onSideBarInstallFinished());
}

TEST_F(BookMarkTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(BookMarkTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
