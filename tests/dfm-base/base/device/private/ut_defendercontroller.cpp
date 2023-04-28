// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <dfm-base/base/device/private/defendercontroller.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
class UT_DefenderController : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_DefenderController, IsScanning)
{
    stub.set_lamda(&DefenderController::start, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DefenderController::getScanningPaths, [] {
        __DBG_STUB_INVOKE__
        return QList<QUrl> { QUrl::fromLocalFile("/home"), QUrl::fromLocalFile("/media") };
    });

    // test isSacnning(const QUrl &)
    EXPECT_TRUE(DefenderController::instance().isScanning(QUrl::fromLocalFile("/home")));

    // test isScanning(const QList<QUrl> &)
    EXPECT_FALSE(DefenderController::instance().isScanning(QList<QUrl>()));
    EXPECT_TRUE(DefenderController::instance().isScanning({ QUrl::fromLocalFile("/home") }));
}
// 1060 compiling issues, todo xushitong， 需要考虑Qt 版本兼容性
//TEST_F(UT_DefenderController, StopScanning)
//{
//    stub.set_lamda(&DefenderController::start, [] { __DBG_STUB_INVOKE__ });
//    QList<QUrl> scanningPaths;
//    stub.set_lamda(&DefenderController::getScanningPaths, [&] { __DBG_STUB_INVOKE__ return scanningPaths; });

//    EXPECT_TRUE(DefenderController::instance().stopScanning(QUrl::fromLocalFile("/home")));

//    scanningPaths.append(QUrl::fromLocalFile("/home"));
//    auto asyncCall_QString_QString = static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, QString &&)>(&QDBusInterface::asyncCall);
//    stub.set_lamda(asyncCall_QString_QString, [] { __DBG_STUB_INVOKE__ return QDBusPendingCall::fromError(QDBusError()); });
//    auto isScanning_QList = static_cast<bool (DefenderController::*)(const QList<QUrl> &)>(&DefenderController::isScanning);

//    bool isScanning = true;
//    stub.set_lamda(isScanning_QList, [&] { __DBG_STUB_INVOKE__ return isScanning; });
//    EXPECT_FALSE(DefenderController::instance().stopScanning(QUrl::fromLocalFile("/home")));
//    isScanning = false;
//    EXPECT_TRUE(DefenderController::instance().stopScanning(QUrl::fromLocalFile("/home")));
//}

TEST_F(UT_DefenderController, ScanningUSBPathsChanged)
{
    DefenderController::instance().scanningPaths.append(QUrl::fromLocalFile("/home"));
    EXPECT_FALSE(DefenderController::instance().scanningPaths.isEmpty());
    EXPECT_NO_FATAL_FAILURE(DefenderController::instance().scanningUsbPathsChanged({ "/media" }););
    EXPECT_FALSE(DefenderController::instance().scanningPaths.contains(QUrl::fromLocalFile("/home")));
    EXPECT_TRUE(DefenderController::instance().scanningPaths.contains(QUrl::fromLocalFile("/media")));
}

TEST_F(UT_DefenderController, GetScanningPaths)
{
    DefenderController::instance().scanningPaths.clear();
    EXPECT_TRUE(DefenderController::instance().scanningPaths.isEmpty());

    DefenderController::instance().scanningPaths.append(QUrl::fromLocalFile("/home"));
    EXPECT_FALSE(DefenderController::instance().getScanningPaths(QUrl::fromLocalFile("/home")).isEmpty());
    EXPECT_TRUE(DefenderController::instance().getScanningPaths(QUrl::fromLocalFile("/home")).contains(QUrl::fromLocalFile("/home")));
}

TEST_F(UT_DefenderController, Start)
{
    stub.set_lamda(&QDBusInterface::property, [] { __DBG_STUB_INVOKE__ return QVariant::fromValue<QString>("/home"); });
    EXPECT_NO_FATAL_FAILURE(DefenderController::instance().start());
    EXPECT_FALSE(DefenderController::instance().interface.isNull());
    EXPECT_FALSE(DefenderController::instance().scanningPaths.isEmpty());
    EXPECT_TRUE(DefenderController::instance().scanningPaths.contains(QUrl::fromLocalFile("/home")));
}
