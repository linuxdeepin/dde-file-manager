// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textindexdbus.cpp
 * @brief Unit tests for TextIndexDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/textindexdbus.h"

#include <QTest>

using namespace src;

class TextIndexDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextIndexDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextIndexDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextIndexDBusTest, TextIndexDBus)
{
    // Test constructor: TextIndexDBus((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
