// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatormenuscene.cpp
 * @brief Unit tests for FileOperatorMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/fileoperatormenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class FileOperatorMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}
