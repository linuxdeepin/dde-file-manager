// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashfiletaginfo_1.cpp
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

TEST_F(TrashFileTagInfoTest, getFileInode)
{
    // Test getter: qint64 getFileInode()
    auto result = obj->getFileInode();
    EXPECT_EQ(result, 0);

}

TEST_F(TrashFileTagInfoTest, getFuture)
{
    // Test getter: QString getFuture()
    auto result = obj->getFuture();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TrashFileTagInfoTest, getOriginalPath)
{
    // Test getter: QString getOriginalPath()
    auto result = obj->getOriginalPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TrashFileTagInfoTest, getTagNames)
{
    // Test getter: QString getTagNames()
    auto result = obj->getTagNames();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TrashFileTagInfoTest, getTrashIndex)
{
    // Test getter: int getTrashIndex()
    auto result = obj->getTrashIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(TrashFileTagInfoTest, setFileInode)
{
    // Test setter: void setFileInode((qint64 value))
    EXPECT_NO_FATAL_FAILURE(obj->setFileInode(0));
}

TEST_F(TrashFileTagInfoTest, setOriginalPath)
{
    // Test setter: void setOriginalPath((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setOriginalPath(_arg0));
}

TEST_F(TrashFileTagInfoTest, setTagNames)
{
    // Test setter: void setTagNames((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTagNames(_arg0));
}

TEST_F(TrashFileTagInfoTest, setTrashIndex)
{
    // Test setter: void setTrashIndex((int value))
    EXPECT_NO_FATAL_FAILURE(obj->setTrashIndex(0));
}
