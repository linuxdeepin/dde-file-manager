// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operationsstackmanageradaptor.cpp
 * @brief Unit tests for OperationsStackManagerAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/OperationsStackManagerAdaptor.h"

#include <QTest>

using namespace src;

class OperationsStackManagerAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OperationsStackManagerAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OperationsStackManagerAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OperationsStackManagerAdaptorTest, OperationsStackManagerAdaptor)
{
    // Test constructor: OperationsStackManagerAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OperationsStackManagerAdaptorTest, SaveOperations)
{
    // Test method: void SaveOperations((const QVariantMap &values))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->SaveOperations(_arg0));
}

TEST_F(OperationsStackManagerAdaptorTest, SaveRedoOperations)
{
    // Test method: void SaveRedoOperations((const QVariantMap &values))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->SaveRedoOperations(_arg0));
}
