// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filegroupdata.cpp
 * @brief Unit tests for FileGroupData Mid-priority methods (dfmplugin-workspace)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "groups/filegroupdata.h"

using namespace dfmplugin_workspace;

class FileGroupDataTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(FileGroupDataTest, addFile)
{
    // Instance method addFile
    FileGroupData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.addFile({}); });
}

TEST_F(FileGroupDataTest, clear)
{
    // Instance method clear
    FileGroupData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.clear(); });
}

TEST_F(FileGroupDataTest, getHeaderText)
{
    // Instance method getHeaderText
    FileGroupData obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.getHeaderText(); (void)r; });
}

TEST_F(FileGroupDataTest, isEmpty)
{
    // Instance method isEmpty
    FileGroupData obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isEmpty(); });
    (void)result;
}

TEST_F(FileGroupDataTest, removeFile)
{
    // Instance method removeFile
    FileGroupData obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.removeFile(QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(FileGroupDataTest, updateFileCount)
{
    // Instance method updateFileCount
    FileGroupData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.updateFileCount(); });
}
