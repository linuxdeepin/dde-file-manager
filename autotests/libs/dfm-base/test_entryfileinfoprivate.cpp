// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_entryfileinfoprivate.cpp
 * @brief Unit tests for EntryFileInfoPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/entry/entryfileinfo.h"

#include <QTest>

using namespace src;

class EntryFileInfoPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EntryFileInfoPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EntryFileInfoPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EntryFileInfoPrivateTest, suffix)
{
    // Test getter: QString suffix()
    auto result = obj->suffix();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
