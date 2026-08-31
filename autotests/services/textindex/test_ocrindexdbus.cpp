// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrindexdbus.cpp
 * @brief Unit tests for OcrIndexDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/ocrindexdbus.h"

#include <QTest>

using namespace src;

class OcrIndexDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrIndexDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrIndexDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrIndexDBusTest, CreateIndexTask)
{
    // Test method: bool CreateIndexTask((const QStringList &paths, const QVariantMap &options))
    QStringList _arg0{};
    QVariantMap _arg1{};
    auto result = obj->CreateIndexTask(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OcrIndexDBusTest, GetLastUpdateTime)
{
    // Test getter: QString GetLastUpdateTime()
    auto result = obj->GetLastUpdateTime();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OcrIndexDBusTest, IndexDatabaseExists)
{
    // Test bool getter: IndexDatabaseExists()
    bool result = obj->IndexDatabaseExists();
    EXPECT_FALSE(result);

}

TEST_F(OcrIndexDBusTest, Init)
{
    // Test method: void Init(())
    EXPECT_NO_FATAL_FAILURE(obj->Init());
}

TEST_F(OcrIndexDBusTest, OcrIndexDBus)
{
    // Test constructor: OcrIndexDBus((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OcrIndexDBusTest, ProcessFileChanges)
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

TEST_F(OcrIndexDBusTest, ProcessFileMoves)
{
    // Test method: bool ProcessFileMoves((const QHash<QString, QString> &movedFiles))
    QHash<QString, QString> _arg0{};
    auto result = obj->ProcessFileMoves(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OcrIndexDBusTest, SetEnabled)
{
    // Test setter: void SetEnabled((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->SetEnabled(false));
}

TEST_F(OcrIndexDBusTest, cleanup)
{
    // Test method: void cleanup(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanup());
}
