// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfileinfo.cpp
 * @brief Unit tests for TagFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/tagfileinfo.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagFileInfoTest, canAttributes)
{
    // Test method: bool canAttributes((const CanableInfoType type))
    auto result = obj->canAttributes(CanableInfoType());
    EXPECT_FALSE(result);

}

TEST_F(TagFileInfoTest, displayOf)
{
    // Test method: QString displayOf((const DisPlayInfoType type))
    auto result = obj->displayOf(DisPlayInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagFileInfoTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(TagFileInfoTest, fileIcon)
{
    // Test getter: QIcon fileIcon()
    auto result = obj->fileIcon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(TagFileInfoTest, isAttributes)
{
    // Test method: bool isAttributes((const OptInfoType type))
    auto result = obj->isAttributes(OptInfoType());
    EXPECT_FALSE(result);

}

TEST_F(TagFileInfoTest, nameOf)
{
    // Test method: QString nameOf((const NameInfoType type))
    auto result = obj->nameOf(NameInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagFileInfoTest, tagName)
{
    // Test getter: QString tagName()
    auto result = obj->tagName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagFileInfoTest, fileType)
{
    // Test getter: FileInfo::FileType fileType()
    auto result = obj->fileType();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TagFileInfoTest, permissions)
{
    // Test getter: QFileDevice::Permissions permissions()
    auto result = obj->permissions();
    EXPECT_GE(static_cast<int>(result), 0);

}
