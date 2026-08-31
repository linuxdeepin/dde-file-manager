// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_commonentryfileentity.cpp
 * @brief Unit tests for CommonEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/commonentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class CommonEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CommonEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CommonEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CommonEntryFileEntityTest, extraProperties)
{
    // Test getter: QVariantHash extraProperties()
    auto result = obj->extraProperties();
    EXPECT_TRUE(result.isEmpty());

}
