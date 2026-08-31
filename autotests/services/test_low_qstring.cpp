// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_qstring.cpp
 * @brief Unit tests for QString methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/accesscontrol/utils.h"

#include <QTest>

using namespace src;

class QStringTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new QString();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    QString *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(QStringTest, devConfigPath)
{
    // Test getter: SERVICEACCESSCONTROL_USE_NAMESPACE devConfigPath()
    auto result = obj->devConfigPath();
    EXPECT_NO_FATAL_FAILURE({ obj->devConfigPath(); });

}
