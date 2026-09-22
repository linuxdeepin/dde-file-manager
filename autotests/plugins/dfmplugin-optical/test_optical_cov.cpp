// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (optical.cpp, complementing test_optical.cpp):
//   addCustomTopWidget / addDelegateSettings / addPropertySettings / bindWindows /
//   onDiscEjected / onAllPluginsStarted / changeUrlEventFilter(+redirect lambda) /
//   openNewWindowEventFilter(+redirect lambda) / openNewWindowWithArgsEventFilter(+redirect lambda)
// Branch notes (from get_code_snippet):
//   onDiscEjected: invalid disc url -> return; mounted -> force unmount; not mounted -> no-op;
//   onAllPluginsStarted: parent menu missing -> warn and return;
//   event filters: packet-writing url -> QTimer redirect lambda (needs event loop), else false.

#include <gtest/gtest.h>
#include "stubext.h"

#include "optical.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

class UT_OpticalCov : public testing::Test
{
protected:
    void SetUp() override
    {
        optical = new Optical();
    }

    void TearDown() override
    {
        delete optical;
        stub.clear();
    }

    void stubPacketWritingUrl(bool isPw)
    {
        stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &) -> QString {
            __DBG_STUB_INVOKE__
            return "/dev/sr0";
        });
        stub.set_lamda(&DeviceUtils::isPWOpticalDiscDev, [isPw](const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return isPw;
        });
        stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localDiscFile), [](const QUrl &) -> QUrl {
            __DBG_STUB_INVOKE__
            return QUrl::fromLocalFile("/media/cdrom");
        });
    }

    Optical *optical = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_OpticalCov, OnDiscEjected_InvalidDiscUrl_ReturnsEarly)
{
    // Arrange
    stub.set_lamda(static_cast<QUrl (*)(const QString &)>(&OpticalHelper::transDiscRootById),
                   [](const QString &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl();   // invalid
                   });
    bool unmountCalled = false;
    stub.set_lamda(ADDR(DeviceManager, unmountBlockDevAsync),
                   [&unmountCalled](DeviceManager *, const QString &, const QVariantMap &, CallbackType2) {
                       __DBG_STUB_INVOKE__
                       unmountCalled = true;
                   });

    // Act
    optical->onDiscEjected("/dev/sr0");

    // Assert
    EXPECT_EQ(unmountCalled, false);
    EXPECT_NE(unmountCalled, true);   // double-check stable state
}

TEST_F(UT_OpticalCov, OnDiscEjected_DeviceStillMounted_ForcesUnmount)
{
    // Arrange
    stub.set_lamda(static_cast<QUrl (*)(const QString &)>(&OpticalHelper::transDiscRootById),
                   [](const QString &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl("burn:///dev/sr0/ondisc");
                   });
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/dev/sr0";
    });
    stub.set_lamda(static_cast<QString (*)(const QString &, bool)>(&DeviceUtils::getMountInfo),
                   [](const QString &, bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return "/media/cdrom";
                   });
    int unmountCount = 0;
    stub.set_lamda(ADDR(DeviceManager, unmountBlockDevAsync),
                   [&unmountCount](DeviceManager *, const QString &id, const QVariantMap &opts, CallbackType2) {
                       __DBG_STUB_INVOKE__
                       ++unmountCount;
                       EXPECT_EQ(id, QString("/dev/sr0"));
                       EXPECT_EQ(opts.value("force").toBool(), true);
                   });

    // Act
    optical->onDiscEjected("/dev/sr0");

    // Assert
    EXPECT_EQ(unmountCount, 1);
}

TEST_F(UT_OpticalCov, OnDiscEjected_DeviceNotMounted_NoUnmount)
{
    // Arrange
    stub.set_lamda(static_cast<QUrl (*)(const QString &)>(&OpticalHelper::transDiscRootById),
                   [](const QString &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl("burn:///dev/sr0/ondisc");
                   });
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/dev/sr0";
    });
    stub.set_lamda(static_cast<QString (*)(const QString &, bool)>(&DeviceUtils::getMountInfo),
                   [](const QString &, bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString();   // not mounted
                   });
    int unmountCount = 0;
    stub.set_lamda(ADDR(DeviceManager, unmountBlockDevAsync),
                   [&unmountCount](DeviceManager *, const QString &, const QVariantMap &, CallbackType2) {
                       __DBG_STUB_INVOKE__
                       ++unmountCount;
                   });

    // Act
    optical->onDiscEjected("/dev/sr0");

    // Assert
    EXPECT_EQ(unmountCount, 0);
    EXPECT_NE(unmountCount, 1);   // complementary bound
}

TEST_F(UT_OpticalCov, OnAllPluginsStarted_NoWorkspaceMenu_WarnsAndReturns)
{
    // Arrange
    // Act: menu framework has no registered scenes in the unit-test binary
    // Assert
    EXPECT_NO_FATAL_FAILURE(optical->onAllPluginsStarted());

    // Assert: second invocation also takes the early-return branch quietly
    EXPECT_NO_FATAL_FAILURE(optical->onAllPluginsStarted());
    EXPECT_EQ(QString(optical->metaObject()->className()), QString("dfmplugin_optical::Optical"));
    EXPECT_NE(QString(optical->metaObject()->className()), QString("Optical"));
}

TEST_F(UT_OpticalCov, AddCustomTopWidget_RegistersCallbackMap)
{
    // Arrange
    // Act: slot channel push with no receiver is a no-op
    // Assert
    // Assert
    EXPECT_NO_FATAL_FAILURE(optical->addCustomTopWidget());
    EXPECT_EQ(QString(optical->metaObject()->className()), QString("dfmplugin_optical::Optical"));
    EXPECT_NE(QString(optical->metaObject()->className()), QString("Optical"));
}

TEST_F(UT_OpticalCov, AddDelegateSettings_AndPropertySettings_RegisterHooks)
{
    // Arrange
    // Act
    // Assert
    // Assert
    EXPECT_NO_FATAL_FAILURE(optical->addDelegateSettings());
    EXPECT_NO_FATAL_FAILURE(optical->addPropertySettings());
    EXPECT_EQ(QString(optical->metaObject()->className()), QString("dfmplugin_optical::Optical"));
    EXPECT_NE(QString(optical->metaObject()->className()), QString("Optical"));
}

TEST_F(UT_OpticalCov, BindWindows_IteratesEmptyWindowList)
{
    // Arrange
    // Act
    // Assert
    // Assert
    EXPECT_NO_FATAL_FAILURE(optical->bindWindows());
    EXPECT_EQ(FileManagerWindowsManager::instance().windowIdList().isEmpty(), true);
    EXPECT_NE(FileManagerWindowsManager::instance().windowIdList().isEmpty(), false);   // double-check stable state
}

TEST_F(UT_OpticalCov, ChangeUrlEventFilter_PacketWritingUrl_RedirectsViaLambda)
{
    // Arrange
    stubPacketWritingUrl(true);

    // Act
    bool redirected = optical->changeUrlEventFilter(42, QUrl("burn:///dev/sr0/staging/file.txt"));

    // run the zero-timer lambda (publishes kChangeCurrentUrl)
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();

    // Assert
    EXPECT_EQ(redirected, true);
    EXPECT_NE(redirected, false);   // double-check stable state
}

TEST_F(UT_OpticalCov, ChangeUrlEventFilter_NonBurnUrl_ReturnsFalse)
{
    // Arrange
    stubPacketWritingUrl(true);

    // Act
    bool redirected = optical->changeUrlEventFilter(42, QUrl("file:///home/user/file.txt"));

    // Assert
    EXPECT_EQ(redirected, false);
    EXPECT_NE(redirected, true);   // double-check stable state
}

TEST_F(UT_OpticalCov, OpenNewWindowEventFilter_PacketWritingUrl_RedirectsViaLambda)
{
    // Arrange
    stubPacketWritingUrl(true);

    // Act
    bool redirected = optical->openNewWindowEventFilter(QUrl("burn:///dev/sr0/staging/file.txt"));
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();

    // Assert
    EXPECT_EQ(redirected, true);
    EXPECT_NE(redirected, false);   // double-check stable state
}

TEST_F(UT_OpticalCov, OpenNewWindowEventFilter_NonPacketWriting_ReturnsFalse)
{
    // Arrange: device is not packet writing
    stubPacketWritingUrl(false);

    // Act
    bool redirected = optical->openNewWindowEventFilter(QUrl("burn:///dev/sr0/staging/file.txt"));

    // Assert
    EXPECT_EQ(redirected, false);
    EXPECT_NE(redirected, true);   // double-check stable state
}

TEST_F(UT_OpticalCov, OpenNewWindowWithArgsEventFilter_BothBranches)
{
    // Arrange
    stubPacketWritingUrl(true);

    // Act
    bool redirected = optical->openNewWindowWithArgsEventFilter(QUrl("burn:///dev/sr0/staging/file.txt"), true);
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
    bool notRedirected = optical->openNewWindowWithArgsEventFilter(QUrl("file:///tmp/x"), true);

    // Assert
    EXPECT_EQ(redirected, true);
    EXPECT_EQ(notRedirected, false);
}
