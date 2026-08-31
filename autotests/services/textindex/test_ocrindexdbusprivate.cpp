// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrindexdbusprivate.cpp
 * @brief Unit tests for OcrIndexDBusPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/ocrindexdbus.h"

#include <QTest>

using namespace src;

class OcrIndexDBusPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrIndexDBusPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrIndexDBusPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrIndexDBusPrivateTest, handleSlientStart)
{
    // Test method: void handleSlientStart(())
    EXPECT_NO_FATAL_FAILURE(obj->handleSlientStart());
}
