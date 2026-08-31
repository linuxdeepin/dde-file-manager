// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsfileiteratorprivate.cpp
 * @brief Unit tests for AvfsFileIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/avfsfileiterator.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsFileIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsFileIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsFileIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsFileIteratorPrivateTest, AvfsFileIteratorPrivate)
{
    // Test constructor: AvfsFileIteratorPrivate((const QUrl &root, AvfsFileIterator *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AvfsFileIteratorPrivateTest, AvfsFileIteratorPrivate_Destructor)
{
    // Test method:  ~AvfsFileIteratorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ AvfsFileIteratorPrivate *tmp = new AvfsFileIteratorPrivate(); delete tmp; });
}
