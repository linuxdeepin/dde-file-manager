// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskencryptadaptor.cpp
 * @brief Unit tests for DiskEncryptAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/DiskEncryptAdaptor.h"

#include <QTest>

using namespace src;

class DiskEncryptAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptAdaptorTest, DiskEncryptAdaptor)
{
    // Test constructor: DiskEncryptAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
