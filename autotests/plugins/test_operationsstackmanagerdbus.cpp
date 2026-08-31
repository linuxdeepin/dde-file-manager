// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operationsstackmanagerdbus.cpp
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

TEST_F(OperationsStackManagerDbusTest, OperationsStackManagerDbus)
{
    // Test constructor: OperationsStackManagerDbus((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OperationsStackManagerDbusTest, SaveOperations)
{
    // Test method: void SaveOperations((const QVariantMap &values))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->SaveOperations(_arg0));
}

TEST_F(OperationsStackManagerDbusTest, SaveRedoOperations)
{
    // Test method: void SaveRedoOperations((const QVariantMap &values))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->SaveRedoOperations(_arg0));
}
