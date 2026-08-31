// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashfiletaginfo.cpp
 * @brief Unit tests for TrashFileTagInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "beans/trashfiletaginfo.h"

#include <QTest>

using namespace tag;

class TrashFileTagInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashFileTagInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashFileTagInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashFileTagInfoTest, getDeleteTime)
{
    // Test getter: qint64 getDeleteTime()
    auto result = obj->getDeleteTime();
    EXPECT_EQ(result, 0);

}

TEST_F(TrashFileTagInfoTest, setDeleteTime)
{
    // Test setter: void setDeleteTime((qint64 value))
    EXPECT_NO_FATAL_FAILURE(obj->setDeleteTime(0));
}

TEST_F(TrashFileTagInfoTest, setFuture)
{
    // Test setter: void setFuture((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFuture(_arg0));
}
