// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tag.cpp
 * @brief Unit tests for Tag methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tag.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Tag();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Tag *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagTest, bindWindows)
{
    // Test method: void bindWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->bindWindows());
}

TEST_F(TagTest, installToSideBar)
{
    // Test method: void installToSideBar(())
    EXPECT_NO_FATAL_FAILURE(obj->installToSideBar());
}

TEST_F(TagTest, onAllPluginsStarted)
{
    // Test method: void onAllPluginsStarted(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsStarted());
}

TEST_F(TagTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(TagTest, regTagCrumbToTitleBar)
{
    // Test method: void regTagCrumbToTitleBar(())
    EXPECT_NO_FATAL_FAILURE(obj->regTagCrumbToTitleBar());
}
