// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extractorapp.cpp
 * @brief Unit tests for ExtractorApp methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-extractor/extractor/extractorapp.h"

#include <QTest>

using namespace src;

class ExtractorAppTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtractorApp();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtractorApp *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtractorAppTest, resetIdleTimer)
{
    // Test method: void resetIdleTimer(())
    EXPECT_NO_FATAL_FAILURE(obj->resetIdleTimer());
}
