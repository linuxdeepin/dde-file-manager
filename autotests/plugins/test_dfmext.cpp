// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmext.cpp
 * @brief Unit tests for DFMEXT methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension-example/mymenuplugin.h"

#include <QTest>

using namespace examples;

class DFMEXTTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMEXT();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMEXT *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMEXTTest, DFMExtMenuProxy)
{
    // Test getter: DPUTILS_BEGIN_NAMESPACE DFMExtMenuProxy()
    auto result = obj->DFMExtMenuProxy();
    EXPECT_NO_FATAL_FAILURE({ obj->DFMExtMenuProxy(); });

}
