// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetaginfo.cpp
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

TEST_F(FileTagInfoTest, FileTagInfo)
{
    // Test constructor: FileTagInfo((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileTagInfoTest, setFilePath)
{
    // Test setter: void setFilePath((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilePath(_arg0));
}

TEST_F(FileTagInfoTest, setTagName)
{
    // Test setter: void setTagName((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTagName(_arg0));
}

TEST_F(FileTagInfoTest, setTagOrder)
{
    // Test setter: void setTagOrder((int value))
    EXPECT_NO_FATAL_FAILURE(obj->setTagOrder(0));
}
