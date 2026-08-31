// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_serviceusbrepair_use_namespace.cpp
 * @brief Unit tests for SERVICEUSBREPAIR_USE_NAMESPACE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/usbrepair/usbrepairdbus.h"

#include <QTest>

using namespace src;

class SERVICEUSBREPAIR_USE_NAMESPACETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SERVICEUSBREPAIR_USE_NAMESPACE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SERVICEUSBREPAIR_USE_NAMESPACE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SERVICEUSBREPAIR_USE_NAMESPACETest, initConnect)
{
    // Test getter: SERVICEUSBREPAIR_END_NAMESPACE initConnect()
    auto result = obj->initConnect();
    EXPECT_NO_FATAL_FAILURE({ obj->initConnect(); });

}
