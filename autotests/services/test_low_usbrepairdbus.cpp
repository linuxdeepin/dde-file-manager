// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_usbrepairdbus.cpp
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

TEST_F(UsbRepairDBusTest, CancelRepair)
{
    // Test method: bool CancelRepair((const QString &devicePath))
    QString _arg0{};
    auto result = obj->CancelRepair(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairDBusTest, cleanup)
{
    // Test method: void cleanup(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanup());
}
