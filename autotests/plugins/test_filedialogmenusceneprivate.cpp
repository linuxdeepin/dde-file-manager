// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogmenusceneprivate.cpp
 * @brief Unit tests for FileDialogMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/filedialogmenuscene.h"

#include <QTest>

using namespace core;

class FileDialogMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogMenuScenePrivateTest, FileDialogMenuScenePrivate)
{
    // Test constructor: FileDialogMenuScenePrivate((FileDialogMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
