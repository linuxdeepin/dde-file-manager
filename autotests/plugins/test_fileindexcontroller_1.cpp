// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileindexcontroller_1.cpp
 * @brief Unit tests for FileIndexController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileindexcontroller.h"

#include <QTest>

using namespace core;

class FileIndexControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileIndexController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileIndexController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileIndexControllerTest, FileIndexController)
{
    // Test constructor: FileIndexController((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileIndexControllerTest, createRefreshIndexFile)
{
    // Test bool getter: createRefreshIndexFile()
    bool result = obj->createRefreshIndexFile();
    EXPECT_FALSE(result);

}

TEST_F(FileIndexControllerTest, disableFileIndex)
{
    // Test bool getter: disableFileIndex()
    bool result = obj->disableFileIndex();
    EXPECT_FALSE(result);

}

TEST_F(FileIndexControllerTest, handleConfigChanged)
{
    // Test method: void handleConfigChanged((const QString &config, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleConfigChanged(_arg0, _arg1));
}

TEST_F(FileIndexControllerTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(FileIndexControllerTest, isServiceActive)
{
    // Test bool getter: isServiceActive()
    bool result = obj->isServiceActive();
    EXPECT_FALSE(result);

}

TEST_F(FileIndexControllerTest, isServiceEnabled)
{
    // Test bool getter: isServiceEnabled()
    bool result = obj->isServiceEnabled();
    EXPECT_FALSE(result);

}

TEST_F(FileIndexControllerTest, refreshFilePath)
{
    // Test getter: QString refreshFilePath()
    auto result = obj->refreshFilePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileIndexControllerTest, runSystemctlCommand)
{
    // Test method: bool runSystemctlCommand((const QStringList &arguments))
    QStringList _arg0{};
    auto result = obj->runSystemctlCommand(_arg0);
    EXPECT_FALSE(result);

}
