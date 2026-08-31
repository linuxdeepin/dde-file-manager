// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_usbrepairworker.cpp
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

TEST_F(UsbRepairWorkerTest, blockObjPathFromDevice)
{
    // Test method: QString blockObjPathFromDevice((const QString &devicePath))
    QString _arg0{};
    auto result = obj->blockObjPathFromDevice(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UsbRepairWorkerTest, cancelRepair)
{
    // Test method: bool cancelRepair((const QString &devicePath, const QString &callerBusName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->cancelRepair(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairWorkerTest, checkAuthorization)
{
    // Test method: bool checkAuthorization((const QString &callerBusName))
    QString _arg0{};
    auto result = obj->checkAuthorization(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairWorkerTest, isRepairing)
{
    // Test method: bool isRepairing((const QString &devicePath))
    QString _arg0{};
    auto result = obj->isRepairing(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairWorkerTest, isUsbBlockDevice)
{
    // Test method: bool isUsbBlockDevice((const QString &devicePath))
    QString _arg0{};
    auto result = obj->isUsbBlockDevice(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairWorkerTest, onFsckErrorOccurred)
{
    // Test method: void onFsckErrorOccurred((QProcess::ProcessError error))
    EXPECT_NO_FATAL_FAILURE(obj->onFsckErrorOccurred(QProcess::ProcessError()));
}

TEST_F(UsbRepairWorkerTest, onFsckReadyRead)
{
    // Test method: void onFsckReadyRead(())
    EXPECT_NO_FATAL_FAILURE(obj->onFsckReadyRead());
}

TEST_F(UsbRepairWorkerTest, onFsckTimeout)
{
    // Test method: void onFsckTimeout(())
    EXPECT_NO_FATAL_FAILURE(obj->onFsckTimeout());
}

TEST_F(UsbRepairWorkerTest, umountDevice)
{
    // Test method: bool umountDevice((const QString &devicePath))
    QString _arg0{};
    auto result = obj->umountDevice(_arg0);
EXPECT_FALSE(result);

}

TEST_F(UsbRepairWorkerTest, validateDevicePath)
{
    // Test method: bool validateDevicePath((const QString &devicePath))
    QString _arg0{};
    auto result = obj->validateDevicePath(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairWorkerTest, UsbRepairWorker_Destructor)
{
    // Test method:  ~UsbRepairWorker(())
    EXPECT_NO_FATAL_FAILURE({ UsbRepairWorker *tmp = new UsbRepairWorker(); delete tmp; });
}
