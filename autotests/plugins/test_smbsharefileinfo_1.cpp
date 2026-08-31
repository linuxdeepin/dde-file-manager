// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbsharefileinfo_1.cpp
 * @brief Unit tests for SmbShareFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileinfo/smbsharefileinfo.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbShareFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbShareFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbShareFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbShareFileInfoTest, canAttributes)
{
    // Test method: bool canAttributes((const CanableInfoType type))
    auto result = obj->canAttributes(CanableInfoType());
    EXPECT_FALSE(result);

}

TEST_F(SmbShareFileInfoTest, displayOf)
{
    // Test method: QString displayOf((const DisPlayInfoType type))
    auto result = obj->displayOf(DisPlayInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SmbShareFileInfoTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(SmbShareFileInfoTest, fileIcon)
{
    // Test getter: QIcon fileIcon()
    auto result = obj->fileIcon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(SmbShareFileInfoTest, nameOf)
{
    // Test method: QString nameOf((const NameInfoType type))
    auto result = obj->nameOf(NameInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SmbShareFileInfoTest, SmbShareFileInfo_Destructor)
{
    // Test method:  ~SmbShareFileInfo(())
    EXPECT_NO_FATAL_FAILURE({ SmbShareFileInfo *tmp = new SmbShareFileInfo(); delete tmp; });
}
