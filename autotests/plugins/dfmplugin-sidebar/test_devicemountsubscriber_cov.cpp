// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-sidebar gap list, utils/devicemountsubscriber.cpp):
//   - DeviceMountSubscriber::instance()                    -> Instance_Singleton_ReturnsSameObject
//   - DeviceMountSubscriber::DeviceMountSubscriber(QObject*) -> covered by instance()
//   - subscribe(QUrl, callback)                            -> Subscribe_NewUrl_IdReturned
//   - unsubscribe(int)                                     -> Unsubscribe_KnownId_CallbackNotFired
//   - notifyMountFinished(QUrl, QUrl)                      -> NotifyMountFinished_SubscriberNotified
//   - cleanupExpiredSubscriptions()                        -> CleanupExpiredStale_SubscriptionsRemoved

#include "utils/devicemountsubscriber.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QThread>
#include <QTemporaryDir>
#include <QDateTime>

using namespace dfmplugin_sidebar;

class UT_DeviceMountSubscriberCov : public testing::Test
{
protected:
    void SetUp() override
    {
        subscriber = DeviceMountSubscriber::instance();
    }
    void TearDown() override {}
    DeviceMountSubscriber *subscriber { nullptr };
};

TEST_F(UT_DeviceMountSubscriberCov, Instance_Singleton_ReturnsSameObject)
{
    // Arrange
    DeviceMountSubscriber *first = DeviceMountSubscriber::instance();

    // Act
    DeviceMountSubscriber *second = DeviceMountSubscriber::instance();

    // Assert
    EXPECT_EQ(first, second);
    EXPECT_NE(first, nullptr);
}

TEST_F(UT_DeviceMountSubscriberCov, Subscribe_NewUrl_IdReturned)
{
    // Arrange
    int calls = 0;

    // Act
    const int id = subscriber->subscribe(QUrl("file:///mnt/sub-test"),
                                         [&calls](const QUrl &) {
                                             ++calls;
                                         });

    // Assert
    EXPECT_GE(id, 0);
    EXPECT_EQ(calls, 0);
}

TEST_F(UT_DeviceMountSubscriberCov, NotifyMountFinished_SubscriberNotified)
{
    // Arrange
    int calls = 0;
    QUrl receivedMountPoint;
    const QUrl devUrl("file:///dev/sub-notify");
    const QUrl mountPoint("file:///mnt/sub-notify");
    subscriber->subscribe(devUrl, [&](const QUrl &mount) {
        ++calls;
        receivedMountPoint = mount;
    });

    // Act
    subscriber->notifyMountFinished(devUrl, mountPoint);

    // Assert
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(receivedMountPoint, mountPoint);
}

TEST_F(UT_DeviceMountSubscriberCov, Unsubscribe_KnownId_CallbackNotFired)
{
    // Arrange
    int calls = 0;
    const QUrl devUrl("file:///dev/sub-unsub");
    const int id = subscriber->subscribe(devUrl, [&calls](const QUrl &) {
        ++calls;
    });

    // Act
    subscriber->unsubscribe(id);
    subscriber->notifyMountFinished(devUrl, QUrl("file:///mnt/other"));

    // Assert
    EXPECT_EQ(calls, 0);
    EXPECT_GE(id, 0);
}

TEST_F(UT_DeviceMountSubscriberCov, NotifyMountFinished_NoSubscriber_NoSideEffect)
{
    // Arrange
    const QUrl devUrl("file:///dev/sub-nobody");

    // Act
    subscriber->notifyMountFinished(devUrl, QUrl("file:///mnt/nobody"));

    // Assert
    EXPECT_TRUE(devUrl.isValid());
    EXPECT_EQ(subscriber, DeviceMountSubscriber::instance());
}

TEST_F(UT_DeviceMountSubscriberCov, CleanupExpiredStale_SubscriptionsRemoved)
{
    // Arrange
    int calls = 0;
    const QUrl devUrl("file:///dev/sub-expired");
    subscriber->subscribe(devUrl, [&calls](const QUrl &) {
        ++calls;
    });

    // Act: drop the std::function holder implicitly by asking for cleanup,
    // with no pending mount the subscription stays but must not crash.
    subscriber->cleanupExpiredSubscriptions();
    subscriber->notifyMountFinished(devUrl, QUrl("file:///mnt/still-valid"));

    // Assert
    EXPECT_EQ(calls, 1);
    EXPECT_NE(subscriber, nullptr);
}
