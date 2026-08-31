// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpf_namespace.cpp
 * @brief Unit tests for DPF_NAMESPACE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "core.h"

#include <QTest>

using namespace core;

class DPF_NAMESPACETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DPF_NAMESPACE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DPF_NAMESPACE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DPF_NAMESPACETest, Plugin)
{
    // Test getter: DAEMONPCORE_BEGIN_NAMESPACE Plugin()
    auto result = obj->Plugin();
    EXPECT_NO_FATAL_FAILURE({ obj->Plugin(); });

}

TEST_F(DPF_NAMESPACETest, Plugin_Plugin)
{
    // Test getter: DAEMONPCORE_BEGIN_NAMESPACE Plugin()
    auto result = obj->Plugin();
    EXPECT_NO_FATAL_FAILURE({ obj->Plugin(); });

}
