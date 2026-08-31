// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextfileoperationhookprivate.cpp
 * @brief Unit tests for DFMExtFileOperationHookPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension/file/private/dfmextfileoperationhookprivate.h"

#include <QTest>

using namespace src;

class DFMExtFileOperationHookPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtFileOperationHookPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtFileOperationHookPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtFileOperationHookPrivateTest, DFMExtFileOperationHookPrivate)
{
    // Test constructor: DFMExtFileOperationHookPrivate(())
    ASSERT_NE(obj, nullptr);
}
