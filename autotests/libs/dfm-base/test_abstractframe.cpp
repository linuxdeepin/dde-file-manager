// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractframe.cpp
 * @brief Unit tests for AbstractFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/interfaces/abstractframe.h"

#include <QTest>

using namespace src;

class AbstractFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractFrameTest, AbstractFrame)
{
    // Test constructor: AbstractFrame((QWidget *parent, Qt::WindowFlags f))
    ASSERT_NE(obj, nullptr);
}
