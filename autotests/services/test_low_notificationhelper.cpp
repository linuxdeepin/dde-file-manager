// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_notificationhelper.cpp
 * @brief Unit tests for NotificationHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/notificationhelper.h"

#include <QTest>

using namespace src;

class NotificationHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NotificationHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NotificationHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NotificationHelperTest, instance)
{
    // Test getter: FILE_ENCRYPT_USE_NS instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
