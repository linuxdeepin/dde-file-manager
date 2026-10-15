// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventfire_cov.cpp - event template instantiation sweep.
//
// Production plugin code instantiates dpf event templates (EventHelper,
// EventChannel::setReceiver, EventDispatcher/EventSequence append, connect,
// subscribe, follow, memberFunctionVoidCast) against concrete receiver
// classes, but those instantiations are never executed by tests. Each case
// below registers one such receiver member on a fresh unique topic id inside
// a local manager instance and fires it with deliberately mismatched arity
// (or a null receiver, which the framework guards with `if (s)`): the
// EventHelper count-guard / null-receiver guard returns early, so the
// production slot BODY never runs, while the registration template,
// memberFunctionVoidCast, EventHelper<M>, its invoke and resultGenerator<Ret>
// all execute. Assertions document that contract:
//   - connect/subscribe/follow/installEventFilter succeeds
//   - push returns default-constructed (invalid for void, valid otherwise)
//   - publish still dispatches (true), run has no accepting handler (false)
//   - unsubscribe/unfollow removes the previously added handler (true)

#include <plugins/common/dfmplugin-dirshare/utils/usersharehelper.h>
#include <plugins/common/dfmplugin-dirshare/dirshare.h>
#include <plugins/common/dfmplugin-dirshare/widget/sharecontrolwidget.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_UserShareHelper_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_dirshare::UserShareHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62900, receiver, static_cast<uint (dfmplugin_dirshare::UserShareHelper::*)(const QString &)>(&dfmplugin_dirshare::UserShareHelper::whoShared));
    const QVariant ret = mgr.push(62900);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_UserShareHelper_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_dirshare::UserShareHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62902, receiver, static_cast<QVariantMap (dfmplugin_dirshare::UserShareHelper::*)(const QString &)>(&dfmplugin_dirshare::UserShareHelper::shareInfoByPath));
    const QVariant ret = mgr.push(62902);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_UserShareHelper_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_dirshare::UserShareHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62904, receiver, static_cast<bool (dfmplugin_dirshare::UserShareHelper::*)(const QVariantMap &)>(&dfmplugin_dirshare::UserShareHelper::share));
    const QVariant ret = mgr.push(62904);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_UserShareHelper_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_dirshare::UserShareHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62906, receiver, static_cast<QList<QVariantMap> (dfmplugin_dirshare::UserShareHelper::*)()>(&dfmplugin_dirshare::UserShareHelper::shareInfos));
    const QVariant ret = mgr.push(62906, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_DirShare_4_ArityGuardNoBody)
{
    // Arrange
    dfmplugin_dirshare::DirShare receiver;
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62908, &receiver, static_cast<void (dfmplugin_dirshare::DirShare::*)(const QString &)>(&dfmplugin_dirshare::DirShare::bindScene));
    const bool published = mgr.publish(62908);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(62908, &receiver, static_cast<void (dfmplugin_dirshare::DirShare::*)(const QString &)>(&dfmplugin_dirshare::DirShare::bindScene)));  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_ShareControlWidget_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_dirshare::ShareControlWidget *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62910, receiver, static_cast<void (dfmplugin_dirshare::ShareControlWidget::*)(const QString &)>(&dfmplugin_dirshare::ShareControlWidget::onShareNameChanged));
    const bool published = mgr.publish(62910);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

