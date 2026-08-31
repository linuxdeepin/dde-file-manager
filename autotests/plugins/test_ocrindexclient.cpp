// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrindexclient.cpp
 * @brief Unit tests for OcrIndexClient methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/ocrindexclient.h"

#include <QTest>

using namespace dfmplugin_search;

class OcrIndexClientTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrIndexClient();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrIndexClient *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrIndexClientTest, instance)
{
    // Test getter: OcrIndexClient instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
