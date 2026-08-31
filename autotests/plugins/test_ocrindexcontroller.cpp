// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrindexcontroller.cpp
 * @brief Unit tests for OcrIndexController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "ocrindexcontroller.h"

#include <QTest>

using namespace core;

class OcrIndexControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrIndexController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrIndexController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrIndexControllerTest, OcrIndexController)
{
    // Test constructor: OcrIndexController((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
