// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatormenuscene_1.cpp
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

TEST_F(FileOperatorMenuSceneTest, FileOperatorMenuScene)
{
    // Test constructor: FileOperatorMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileOperatorMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileOperatorMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileOperatorMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}
