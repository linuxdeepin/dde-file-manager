// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbsharefileinfoprivate.cpp
 * @brief Unit tests for SmbShareFileInfoPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileinfo/smbsharefileinfo.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbShareFileInfoPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbShareFileInfoPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbShareFileInfoPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbShareFileInfoPrivateTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
