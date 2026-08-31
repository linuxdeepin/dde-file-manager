// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewsorter.cpp
 * @brief Unit tests for FileViewSorter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewsorter.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewSorterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewSorter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewSorter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewSorterTest, sort)
{
    // Test method: QList<QUrl> sort((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->sort(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewSorterTest, sortSeparated)
{
    // Test method: QList<QUrl> sortSeparated((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->sortSeparated(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
