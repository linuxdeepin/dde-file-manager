// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emblemhelper.cpp
 * @brief Unit tests for EmblemHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/emblemhelper.h"

#include <QTest>

using namespace dfmplugin_emblem;

class EmblemHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmblemHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmblemHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmblemHelperTest, clearEmblem)
{
    // Test method: void clearEmblem(())
    EXPECT_NO_FATAL_FAILURE(obj->clearEmblem());
}

TEST_F(EmblemHelperTest, pending)
{
    // Test method: void pending((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->pending(_arg0));
}

TEST_F(EmblemHelperTest, standardEmblem)
{
    // Test method: QIcon standardEmblem((const SystemEmblemType type))
    auto result = obj->standardEmblem(SystemEmblemType());
    EXPECT_TRUE(result.isNull());

}

TEST_F(EmblemHelperTest, systemEmblems)
{
    // Test method: QList<QIcon> systemEmblems((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->systemEmblems(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EmblemHelperTest, EmblemHelper)
{
    // Test constructor: EmblemHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
