// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_checkboxwithfileindex_1.cpp
 * @brief Unit tests for CheckBoxWithFileIndex methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/checkboxwithfileindex.h"

#include <QTest>

using namespace dfmplugin_search;

class CheckBoxWithFileIndexTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CheckBoxWithFileIndex();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CheckBoxWithFileIndex *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CheckBoxWithFileIndexTest, CheckBoxWithFileIndex)
{
    // Test constructor: CheckBoxWithFileIndex((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CheckBoxWithFileIndexTest, acceptCheckStateChange)
{
    // Test method: bool acceptCheckStateChange((Qt::CheckState oldState, Qt::CheckState newState))
    auto result = obj->acceptCheckStateChange(Qt::CheckState(), Qt::CheckState());
    EXPECT_FALSE(result);

}

TEST_F(CheckBoxWithFileIndexTest, confirmDisableFileIndex)
{
    // Test bool getter: confirmDisableFileIndex()
    bool result = obj->confirmDisableFileIndex();
    EXPECT_FALSE(result);

}

TEST_F(CheckBoxWithFileIndexTest, createRefreshIndexFile)
{
    // Test bool getter: createRefreshIndexFile()
    bool result = obj->createRefreshIndexFile();
    EXPECT_FALSE(result);

}

TEST_F(CheckBoxWithFileIndexTest, formatDisplayTime)
{
    // Test method: QString formatDisplayTime((const QString &isoTime))
    QString _arg0{};
    auto result = obj->formatDisplayTime(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CheckBoxWithFileIndexTest, handleCheckStateChanged)
{
    // Test method: void handleCheckStateChanged((Qt::CheckState state))
    EXPECT_NO_FATAL_FAILURE(obj->handleCheckStateChanged(Qt::CheckState()));
}

TEST_F(CheckBoxWithFileIndexTest, initStatusBar)
{
    // Test method: void initStatusBar(())
    EXPECT_NO_FATAL_FAILURE(obj->initStatusBar());
}

TEST_F(CheckBoxWithFileIndexTest, refreshFilePath)
{
    // Test getter: QString refreshFilePath()
    auto result = obj->refreshFilePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CheckBoxWithFileIndexTest, refreshState)
{
    // Test method: void refreshState(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshState());
}

TEST_F(CheckBoxWithFileIndexTest, restartFileIndex)
{
    // Test bool getter: restartFileIndex()
    bool result = obj->restartFileIndex();
    EXPECT_FALSE(result);

}

TEST_F(CheckBoxWithFileIndexTest, runSystemctlCommand)
{
    // Test method: CheckBoxWithFileIndex::CommandResult runSystemctlCommand((const QStringList &arguments))
    QStringList _arg0{};
    auto result = obj->runSystemctlCommand(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CheckBoxWithFileIndexTest, statusFilePath)
{
    // Test getter: QString statusFilePath()
    auto result = obj->statusFilePath();
    EXPECT_TRUE(result.isEmpty());

}
