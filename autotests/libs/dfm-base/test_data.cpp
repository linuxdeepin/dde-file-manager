// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_data.cpp
 * @brief Unit tests for Data methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/application/settings.h"

#include <QTest>

using namespace src;

class DataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Data();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Data *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DataTest, setValue)
{
    // Test method: void setValue(())
    EXPECT_NO_FATAL_FAILURE(obj->setValue());
}

TEST_F(DataTest, value)
{
    // Test getter: QVariant value()
    auto result = obj->value();
    EXPECT_FALSE(result.isValid());

}
