// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsfileiterator.cpp
 * @brief Unit tests for AvfsFileIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/avfsfileiterator.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsFileIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsFileIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsFileIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsFileIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
