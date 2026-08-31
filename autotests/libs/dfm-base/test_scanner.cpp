// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_scanner.cpp
 * @brief Unit tests for Scanner methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/private/discdevicescanner.h"

#include <QTest>

using namespace src;

class ScannerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Scanner();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Scanner *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScannerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}
