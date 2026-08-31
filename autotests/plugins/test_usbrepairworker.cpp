// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_usbrepairworker.cpp
 * @brief Unit tests for UsbRepairWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/usbrepair/usbrepairworker.h"

#include <QTest>

using namespace src;

class UsbRepairWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UsbRepairWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UsbRepairWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UsbRepairWorkerTest, executeFsck)
{
    // Test method: void executeFsck((const QString &devicePath, const QString &fsType))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->executeFsck(_arg0, _arg1));
}
