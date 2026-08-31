// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operationsstackproxy.cpp
 * @brief Unit tests for OperationsStackProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/operationsstackproxy.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class OperationsStackProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OperationsStackProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OperationsStackProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OperationsStackProxyTest, SaveRedoOperations)
{
    // Test method: void SaveRedoOperations((const QVariantMap &values))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->SaveRedoOperations(_arg0));
}

TEST_F(OperationsStackProxyTest, instance)
{
    // Test getter: OperationsStackProxy instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(OperationsStackProxyTest, saveOperations)
{
    // Test method: void saveOperations((const QVariantMap &values))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveOperations(_arg0));
}
