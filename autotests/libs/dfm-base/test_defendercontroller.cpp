// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_defendercontroller.cpp
 * @brief Unit tests for DefenderController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/private/defendercontroller.h"

#include <QTest>

using namespace src;

class DefenderControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DefenderController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DefenderController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DefenderControllerTest, isScanning)
{
    // Test method: bool isScanning((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->isScanning(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DefenderControllerTest, scanningUsbPathsChanged)
{
    // Test method: void scanningUsbPathsChanged((const QStringList &list))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->scanningUsbPathsChanged(_arg0));
}
