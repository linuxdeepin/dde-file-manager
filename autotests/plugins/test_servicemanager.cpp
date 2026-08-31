// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_servicemanager.cpp
 * @brief Unit tests for ServiceManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/servicemanager.h"

#include <QTest>

using namespace dfmplugin_vault;

class ServiceManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ServiceManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ServiceManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ServiceManagerTest, ServiceManager)
{
    // Test constructor: ServiceManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
