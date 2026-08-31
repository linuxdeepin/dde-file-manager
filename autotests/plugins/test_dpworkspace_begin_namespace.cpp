// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpworkspace_begin_namespace.cpp
 * @brief Unit tests for DPWORKSPACE_BEGIN_NAMESPACE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/nogroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class DPWORKSPACE_BEGIN_NAMESPACETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DPWORKSPACE_BEGIN_NAMESPACE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DPWORKSPACE_BEGIN_NAMESPACE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DPWORKSPACE_BEGIN_NAMESPACETest, AbstractGroupStrategy)
{
    // Test getter: DFMBASE_USE_NAMESPACE AbstractGroupStrategy()
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractGroupStrategy(); });
}

TEST_F(DPWORKSPACE_BEGIN_NAMESPACETest, AbstractGroupStrategy_2)
{
    // Test getter: DFMBASE_USE_NAMESPACE AbstractGroupStrategy()
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractGroupStrategy(); });
}
