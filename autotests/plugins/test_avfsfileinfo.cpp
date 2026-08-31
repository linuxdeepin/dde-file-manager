// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsfileinfo.cpp
 * @brief Unit tests for AvfsFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/avfsfileinfo.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsFileInfoTest, canAttributes)
{
    // Test method: bool canAttributes((const FileInfo::FileCanType type))
    auto result = obj->canAttributes(FileInfo::FileCanType());
    EXPECT_FALSE(result);

}

TEST_F(AvfsFileInfoTest, urlOf)
{
    // Test method: QUrl urlOf((const FileInfo::FileUrlInfoType type))
    auto result = obj->urlOf(FileInfo::FileUrlInfoType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(AvfsFileInfoTest, AvfsFileInfo_Destructor)
{
    // Test method:  ~AvfsFileInfo(())
    EXPECT_NO_FATAL_FAILURE({ AvfsFileInfo *tmp = new AvfsFileInfo(); delete tmp; });
}
