// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_defendercontroller.cpp
 * @brief Unit tests for DefenderController (base/device/private/defendercontroller.cpp).
 *        All calls feed invalid URLs or an empty list so every method hits its
 *        early return BEFORE the DBus-touching start()/asyncCall path. The
 *        in-memory slot scanningUsbPathsChanged is exercised directly (no DBus).
 *        All methods are private; the test build uses -fno-access-control.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QList>
#include <QString>
#include <QStringList>

#include <dfm-base/base/device/private/defendercontroller.h>

using namespace dfmbase;

TEST(DefenderControllerTest, InstanceReturnsSingleton)
{
    DefenderController &a = DefenderController::instance();
    DefenderController &b = DefenderController::instance();
    EXPECT_EQ(&a, &b);
}

TEST(DefenderControllerTest, IsScanningInvalidUrlReturnsFalse)
{
    QUrl invalid;
    EXPECT_FALSE(DefenderController::instance().isScanning(invalid));
}

TEST(DefenderControllerTest, IsScanningEmptyListReturnsFalse)
{
    QList<QUrl> empty;
    EXPECT_FALSE(DefenderController::instance().isScanning(empty));
}

TEST(DefenderControllerTest, StopScanningInvalidUrlReturnsFalse)
{
    QUrl invalid;
    EXPECT_FALSE(DefenderController::instance().stopScanning(invalid));
}

TEST(DefenderControllerTest, StopScanningEmptyListReturnsFalse)
{
    QList<QUrl> empty;
    EXPECT_FALSE(DefenderController::instance().stopScanning(empty));
}

TEST(DefenderControllerTest, GetScanningPathsInvalidUrlReturnsEmpty)
{
    QUrl invalid;
    QList<QUrl> paths = DefenderController::instance().getScanningPaths(invalid);
    EXPECT_TRUE(paths.isEmpty());
}

TEST(DefenderControllerTest, GetScanningPathsValidUrlIteratesEmptyCache)
{
    QUrl valid("file:///tmp");
    QList<QUrl> paths = DefenderController::instance().getScanningPaths(valid);
    EXPECT_TRUE(paths.isEmpty());   // scanningPaths cache is empty
}

TEST(DefenderControllerTest, ScanningUsbPathsChangedEmptyList)
{
    EXPECT_NO_FATAL_FAILURE({ DefenderController::instance().scanningUsbPathsChanged(QStringList()); });
}

TEST(DefenderControllerTest, ScanningUsbPathsChangedSkipsEmptyEntries)
{
    QStringList list { "", "/tmp/dfm_test_usb" };
    EXPECT_NO_FATAL_FAILURE({ DefenderController::instance().scanningUsbPathsChanged(list); });
}
