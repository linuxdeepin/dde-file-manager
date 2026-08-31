// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operationsstackproxy_1.cpp
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

TEST_F(OperationsStackProxyTest, CleanOperationsByUrl)
{
    // Test method: void CleanOperationsByUrl((const QStringList &urls))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->CleanOperationsByUrl(_arg0));
}

TEST_F(OperationsStackProxyTest, OperationsStackProxy)
{
    // Test constructor: OperationsStackProxy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OperationsStackProxyTest, RevocationRedoOperations)
{
    // Test getter: QVariantMap RevocationRedoOperations()
    auto result = obj->RevocationRedoOperations();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperationsStackProxyTest, cleanOperations)
{
    // Test method: void cleanOperations(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanOperations());
}

TEST_F(OperationsStackProxyTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(OperationsStackProxyTest, revocationOperations)
{
    // Test getter: QVariantMap revocationOperations()
    auto result = obj->revocationOperations();
    EXPECT_TRUE(result.isEmpty());

}
