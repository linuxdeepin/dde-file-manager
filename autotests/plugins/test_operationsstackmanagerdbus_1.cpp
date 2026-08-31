// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operationsstackmanagerdbus_1.cpp
 * @brief Unit tests for OperationsStackManagerDbus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "operationsstackmanagerdbus.h"

#include <QTest>

using namespace core;

class OperationsStackManagerDbusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OperationsStackManagerDbus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OperationsStackManagerDbus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OperationsStackManagerDbusTest, CleanOperations)
{
    // Test method: void CleanOperations(())
    EXPECT_NO_FATAL_FAILURE(obj->CleanOperations());
}

TEST_F(OperationsStackManagerDbusTest, CleanOperationsByUrl)
{
    // Test method: void CleanOperationsByUrl((const QStringList &urls))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->CleanOperationsByUrl(_arg0));
}

TEST_F(OperationsStackManagerDbusTest, RevocationOperations)
{
    // Test getter: QVariantMap RevocationOperations()
    auto result = obj->RevocationOperations();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperationsStackManagerDbusTest, RevocationRedoOperations)
{
    // Test getter: QVariantMap RevocationRedoOperations()
    auto result = obj->RevocationRedoOperations();
    EXPECT_TRUE(result.isEmpty());

}
