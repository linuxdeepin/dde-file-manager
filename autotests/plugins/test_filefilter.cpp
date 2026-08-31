// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filefilter.cpp
 * @brief Unit tests for FileFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/filefilter.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileFilterTest, fileDeletedFilter)
{
    // Test method: bool fileDeletedFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fileDeletedFilter(_arg0);
    EXPECT_FALSE(result);

}
