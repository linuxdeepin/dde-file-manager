// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dirsharemenuscene.cpp
 * @brief Unit tests for DirShareMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dirsharemenu/dirsharemenuscene.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class DirShareMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirShareMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirShareMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirShareMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DirShareMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
