// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbsharefileinfo.cpp
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

TEST_F(SmbShareFileInfoTest, isAttributes)
{
    // Test method: bool isAttributes((const OptInfoType type))
    auto result = obj->isAttributes(OptInfoType());
    EXPECT_FALSE(result);

}
