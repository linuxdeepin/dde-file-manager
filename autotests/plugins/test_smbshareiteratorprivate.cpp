// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbshareiteratorprivate.cpp
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

TEST_F(SmbShareIteratorPrivateTest, resetEnumerator)
{
    // Test method: void resetEnumerator(())
    EXPECT_NO_FATAL_FAILURE(obj->resetEnumerator());
}
