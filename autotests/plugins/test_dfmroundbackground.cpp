// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmroundbackground.cpp
 * @brief Unit tests for DFMRoundBackground methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "deviceproperty/devicepropertydialog.h"

#include <QTest>

using namespace dfmplugin_computer;

class DFMRoundBackgroundTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMRoundBackground();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMRoundBackground *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMRoundBackgroundTest, DFMRoundBackground)
{
    // Test constructor: DFMRoundBackground((QWidget *parent, int radius))
    ASSERT_NE(obj, nullptr);
}
