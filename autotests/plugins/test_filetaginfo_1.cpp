// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetaginfo_1.cpp
 * @brief Unit tests for FileTagInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/beans/filetaginfo.h"

#include <QTest>

using namespace src;

class FileTagInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileTagInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileTagInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileTagInfoTest, getFileIndex)
{
    // Test getter: int getFileIndex()
    auto result = obj->getFileIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(FileTagInfoTest, getFilePath)
{
    // Test getter: QString getFilePath()
    auto result = obj->getFilePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagInfoTest, getFuture)
{
    // Test getter: QString getFuture()
    auto result = obj->getFuture();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagInfoTest, getTagName)
{
    // Test getter: QString getTagName()
    auto result = obj->getTagName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagInfoTest, getTagOrder)
{
    // Test getter: int getTagOrder()
    auto result = obj->getTagOrder();
    EXPECT_EQ(result, 0);

}

TEST_F(FileTagInfoTest, setFileIndex)
{
    // Test setter: void setFileIndex((int value))
    EXPECT_NO_FATAL_FAILURE(obj->setFileIndex(0));
}

TEST_F(FileTagInfoTest, setFuture)
{
    // Test setter: void setFuture((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFuture(_arg0));
}
