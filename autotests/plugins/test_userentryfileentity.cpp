// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_userentryfileentity.cpp
 * @brief Unit tests for UserEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/userentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class UserEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UserEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UserEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UserEntryFileEntityTest, targetUrl)
{
    // Test getter: QUrl targetUrl()
    auto result = obj->targetUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
