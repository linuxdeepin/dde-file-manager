// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_textindexdbus.cpp
 * @brief Unit tests for TextIndexDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/textindexdbus.h"

#include <QTest>

using namespace src;

class TextIndexDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextIndexDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextIndexDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextIndexDBusTest, CreateIndexTask)
{
    // Test method: bool CreateIndexTask((const QStringList &paths, const QVariantMap &options))
    QStringList _arg0{};
    QVariantMap _arg1{};
    auto result = obj->CreateIndexTask(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, GetLastUpdateTime)
{
    // Test getter: QString GetLastUpdateTime()
    auto result = obj->GetLastUpdateTime();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TextIndexDBusTest, HasRunningTask)
{
    // Test bool getter: HasRunningTask()
    bool result = obj->HasRunningTask();
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, IndexDatabaseExists)
{
    // Test bool getter: IndexDatabaseExists()
    bool result = obj->IndexDatabaseExists();
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, Init)
{
    // Test method: void Init(())
    EXPECT_NO_FATAL_FAILURE(obj->Init());
}

TEST_F(TextIndexDBusTest, IsEnabled)
{
    // Test bool getter: IsEnabled()
    bool result = obj->IsEnabled();
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, ProcessFileChanges)
{
    // Test method: bool ProcessFileChanges((const QStringList &createdFiles,
                                       const QStringList &modifiedFiles,
                                       const QStringList &deletedFiles))
    QStringList _arg0{};
    QStringList _arg1{};
    QStringList _arg2{};
    auto result = obj->ProcessFileChanges(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, ProcessFileMoves)
{
    // Test method: bool ProcessFileMoves((const QHash<QString, QString> &movedFiles))
    QHash<QString, QString> _arg0{};
    auto result = obj->ProcessFileMoves(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, SetEnabled)
{
    // Test setter: void SetEnabled((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->SetEnabled(false));
}

TEST_F(TextIndexDBusTest, StopCurrentTask)
{
    // Test bool getter: StopCurrentTask()
    bool result = obj->StopCurrentTask();
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, UpdateIndexTask)
{
    // Test method: bool UpdateIndexTask((const QStringList &paths, const QVariantMap &options))
    QStringList _arg0{};
    QVariantMap _arg1{};
    auto result = obj->UpdateIndexTask(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusTest, cleanup)
{
    // Test method: void cleanup(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanup());
}
