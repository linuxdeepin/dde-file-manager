// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogmenuscene.cpp
 * @brief Unit tests for FileDialogMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/filedialogmenuscene.h"

#include <QTest>

using namespace core;

class FileDialogMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogMenuSceneTest, FileDialogMenuScene)
{
    // Test constructor: FileDialogMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileDialogMenuSceneTest, actionFilter)
{
    // Test method: bool actionFilter((AbstractMenuScene *caller, QAction *action))
    auto result = obj->actionFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileDialogMenuSceneTest, findSceneName)
{
    // Test method: QString findSceneName((QAction *act))
    auto result = obj->findSceneName(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileDialogMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
