// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_scanresult.cpp
 * @brief Unit tests for ScanResult methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/filescanner.h"

#include <QTest>

using namespace src;

class ScanResultTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ScanResult();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ScanResult *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScanResultTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(ScanResultTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}
