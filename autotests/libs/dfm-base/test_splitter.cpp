// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_splitter.cpp
 * @brief Unit tests for Splitter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmsplitter/splitter.h"

#include <QTest>

using namespace src;

class SplitterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Splitter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Splitter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SplitterTest, setSplitPosition)
{
    // Test setter: void setSplitPosition((int position))
    EXPECT_NO_FATAL_FAILURE(obj->setSplitPosition(0));
}

TEST_F(SplitterTest, splitPosition)
{
    // Test getter: int splitPosition()
    auto result = obj->splitPosition();
    EXPECT_EQ(result, 0);

}
