// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rsam_1.cpp
 * @brief Unit tests for rsam methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/operator/rsam.h"

#include <QTest>

using namespace dfmplugin_vault;

class rsamTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new rsam();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    rsam *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(rsamTest, privateKeyEncrypt)
{
    // Test method: QString privateKeyEncrypt((const QString &password, const QString &privateKey))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->privateKeyEncrypt(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
