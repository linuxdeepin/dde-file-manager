// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_cifsmounthelperprivate.cpp
 * @brief Unit tests for CifsMountHelperPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/mountcontrol/mounthelpers/cifsmounthelper.h"

#include <QTest>

using namespace src;

class CifsMountHelperPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CifsMountHelperPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CifsMountHelperPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CifsMountHelperPrivateTest, parseIP)
{
    // Test method: QString parseIP((const QString &host, uint16_t port))
    QString _arg0{};
    auto result = obj->parseIP(_arg0, {});
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CifsMountHelperPrivateTest, probeVersion)
{
    // Test method: QString probeVersion((const QString &host, ushort port))
    QString _arg0{};
    auto result = obj->probeVersion(_arg0, {});
    EXPECT_TRUE(result.isEmpty());

}
