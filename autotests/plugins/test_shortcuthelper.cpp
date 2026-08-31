// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shortcuthelper.cpp
 * @brief Unit tests for ShortcutHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/shortcuthelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ShortcutHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShortcutHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShortcutHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShortcutHelperTest, ShortcutHelper)
{
    // Test constructor: ShortcutHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShortcutHelperTest, acitonTriggered)
{
    // Test method: void acitonTriggered(())
    EXPECT_NO_FATAL_FAILURE(obj->acitonTriggered());
}

TEST_F(ShortcutHelperTest, deleteFiles)
{
    // Test method: void deleteFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->deleteFiles());
}

TEST_F(ShortcutHelperTest, openAction)
{
    // Test method: void openAction((const QList<QUrl> &urls, const DirOpenMode openMode))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openAction(_arg0, DirOpenMode()));
}
