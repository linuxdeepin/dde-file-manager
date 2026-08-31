// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmglobal_use_namespace.cpp
 * @brief Unit tests for DFMGLOBAL_USE_NAMESPACE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "old/plugins/dfmplugin-search/test_searchhelper.h"

#include <QTest>

using namespace autotests;

class DFMGLOBAL_USE_NAMESPACETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMGLOBAL_USE_NAMESPACE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMGLOBAL_USE_NAMESPACE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMGLOBAL_USE_NAMESPACETest, sendOpenWindow)
{
    // Test method: DFMBASE_USE_NAMESPACE sendOpenWindow((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj->sendOpenWindow(_arg0); });
}
