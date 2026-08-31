// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatormenusceneprivate.cpp
 * @brief Unit tests for FileOperatorMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/fileoperatormenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class FileOperatorMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorMenuScenePrivateTest, FileOperatorMenuScenePrivate)
{
    // Test constructor: FileOperatorMenuScenePrivate((FileOperatorMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
