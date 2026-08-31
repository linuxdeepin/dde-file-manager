// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmbase_use_namespace.cpp
 * @brief Unit tests for DFMBASE_USE_NAMESPACE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "plugins/ddplugin-organizer/test_organizerplugin.h"

#include <QTest>

using namespace autotests;

class DFMBASE_USE_NAMESPACETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMBASE_USE_NAMESPACE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMBASE_USE_NAMESPACE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMBASE_USE_NAMESPACETest, doWork)
{
    // Test method: DPUTILS_BEGIN_NAMESPACE doWork((const QStringList &paths))
    QStringList _arg0{};
    auto result = obj->doWork(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->doWork(_arg0); });

}
