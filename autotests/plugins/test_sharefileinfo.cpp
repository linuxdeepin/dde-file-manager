// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharefileinfo.cpp
 * @brief Unit tests for ShareFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileinfo/sharefileinfo.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareFileInfoTest, canAttributes)
{
    // Test method: bool canAttributes((const CanableInfoType type))
    auto result = obj->canAttributes(CanableInfoType());
    EXPECT_FALSE(result);

}

TEST_F(ShareFileInfoTest, displayOf)
{
    // Test method: QString displayOf((const DisPlayInfoType type))
    auto result = obj->displayOf(DisPlayInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ShareFileInfoTest, isAttributes)
{
    // Test method: bool isAttributes((const OptInfoType type))
    auto result = obj->isAttributes(OptInfoType());
    EXPECT_FALSE(result);

}

TEST_F(ShareFileInfoTest, nameOf)
{
    // Test method: QString nameOf((const NameInfoType type))
    auto result = obj->nameOf(NameInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ShareFileInfoTest, updateAttributes)
{
    // Test method: void updateAttributes((const QList<FileInfo::FileInfoAttributeID> &types))
    QList<FileInfo::FileInfoAttributeID> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateAttributes(_arg0));
}

TEST_F(ShareFileInfoTest, urlOf)
{
    // Test method: QUrl urlOf((const UrlInfoType type))
    auto result = obj->urlOf(UrlInfoType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(ShareFileInfoTest, ShareFileInfo)
{
    // Test constructor: ShareFileInfo((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}
