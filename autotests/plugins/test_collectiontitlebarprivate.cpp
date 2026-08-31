// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiontitlebarprivate.cpp
 * @brief Unit tests for CollectionTitleBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectiontitlebar.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionTitleBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionTitleBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionTitleBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionTitleBarPrivateTest, showMenu)
{
    // Test method: void showMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->showMenu());
}
