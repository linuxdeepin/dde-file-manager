// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharecontroldbus.cpp
 * @brief Unit tests for ShareControlDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/sharecontrol/sharecontroldbus.h"

#include <QTest>

using namespace src;

class ShareControlDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareControlDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareControlDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareControlDBusTest, CloseSmbShareByShareName)
{
    // Test method: bool CloseSmbShareByShareName((const QString &name, bool show))
    QString _arg0{};
    auto result = obj->CloseSmbShareByShareName(_arg0, false);
    EXPECT_FALSE(result);

}
