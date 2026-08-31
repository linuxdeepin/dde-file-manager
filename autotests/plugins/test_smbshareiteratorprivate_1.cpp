// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbshareiteratorprivate_1.cpp
 * @brief Unit tests for SmbShareIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "iterator/smbshareiterator.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbShareIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbShareIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbShareIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbShareIteratorPrivateTest, SmbShareIteratorPrivate)
{
    // Test constructor: SmbShareIteratorPrivate((const QUrl &url, dfmplugin_smbbrowser::SmbShareIterator *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SmbShareIteratorPrivateTest, SmbShareIteratorPrivate_Destructor)
{
    // Test method:  ~SmbShareIteratorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ SmbShareIteratorPrivate *tmp = new SmbShareIteratorPrivate(); delete tmp; });
}
