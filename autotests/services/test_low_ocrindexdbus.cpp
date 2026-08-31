// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_ocrindexdbus.cpp
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

TEST_F(OcrIndexDBusTest, HasRunningTask)
{
    // Test bool getter: HasRunningTask()
    bool result = obj->HasRunningTask();
    EXPECT_FALSE(result);

}

TEST_F(OcrIndexDBusTest, IsEnabled)
{
    // Test bool getter: IsEnabled()
    bool result = obj->IsEnabled();
    EXPECT_FALSE(result);

}

TEST_F(OcrIndexDBusTest, StopCurrentTask)
{
    // Test bool getter: StopCurrentTask()
    bool result = obj->StopCurrentTask();
    EXPECT_FALSE(result);

}

TEST_F(OcrIndexDBusTest, UpdateIndexTask)
{
    // Test method: bool UpdateIndexTask((const QStringList &paths, const QVariantMap &options))
    QStringList _arg0{};
    QVariantMap _arg1{};
    auto result = obj->UpdateIndexTask(_arg0, _arg1);
    EXPECT_FALSE(result);

}
