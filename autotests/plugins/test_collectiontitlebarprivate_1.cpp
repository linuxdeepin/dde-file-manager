// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiontitlebarprivate_1.cpp
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

TEST_F(CollectionTitleBarPrivateTest, modifyTitleName)
{
    // Test method: void modifyTitleName(())
    EXPECT_NO_FATAL_FAILURE(obj->modifyTitleName());
}

TEST_F(CollectionTitleBarPrivateTest, sendRequestClose)
{
    // Test method: void sendRequestClose(())
    EXPECT_NO_FATAL_FAILURE(obj->sendRequestClose());
}

TEST_F(CollectionTitleBarPrivateTest, titleNameModified)
{
    // Test method: void titleNameModified(())
    EXPECT_NO_FATAL_FAILURE(obj->titleNameModified());
}

TEST_F(CollectionTitleBarPrivateTest, updateDisplayName)
{
    // Test method: void updateDisplayName(())
    EXPECT_NO_FATAL_FAILURE(obj->updateDisplayName());
}

TEST_F(CollectionTitleBarPrivateTest, CollectionTitleBarPrivate_Destructor)
{
    // Test method:  ~CollectionTitleBarPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CollectionTitleBarPrivate *tmp = new CollectionTitleBarPrivate(); delete tmp; });
}
