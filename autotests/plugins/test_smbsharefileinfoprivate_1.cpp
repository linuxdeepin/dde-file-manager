// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbsharefileinfoprivate_1.cpp
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

TEST_F(SmbShareFileInfoPrivateTest, SmbShareFileInfoPrivate)
{
    // Test constructor: SmbShareFileInfoPrivate((SmbShareFileInfo *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SmbShareFileInfoPrivateTest, canDrop)
{
    // Test bool getter: canDrop()
    bool result = obj->canDrop();
    EXPECT_FALSE(result);

}

TEST_F(SmbShareFileInfoPrivateTest, checkAndUpdateNode)
{
    // Test method: void checkAndUpdateNode(())
    EXPECT_NO_FATAL_FAILURE(obj->checkAndUpdateNode());
}

TEST_F(SmbShareFileInfoPrivateTest, SmbShareFileInfoPrivate_Destructor)
{
    // Test method:  ~SmbShareFileInfoPrivate(())
    EXPECT_NO_FATAL_FAILURE({ SmbShareFileInfoPrivate *tmp = new SmbShareFileInfoPrivate(); delete tmp; });
}
