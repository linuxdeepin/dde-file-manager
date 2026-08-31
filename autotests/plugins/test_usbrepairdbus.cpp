// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_usbrepairdbus.cpp
 * @brief Unit tests for UsbRepairDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/usbrepair/usbrepairdbus.h"

#include <QTest>

using namespace src;

class UsbRepairDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UsbRepairDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UsbRepairDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UsbRepairDBusTest, Repair)
{
    // Test method: bool Repair((const QString &devicePath, QString &errorMessage))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->Repair(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairDBusTest, UsbRepairDBus)
{
    // Test constructor: UsbRepairDBus((const char *name, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
