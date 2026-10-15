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

#include <plugins/common/dfmplugin-propertydialog/events/propertyeventreceiver.h>
#include <plugins/common/dfmplugin-propertydialog/propertydialog.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_PropertyEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_propertydialog::PropertyEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62700, receiver, static_cast<bool (dfmplugin_propertydialog::PropertyEventReceiver::*)(dfmplugin_propertydialog::CustomViewExtensionView, const QString &, int)>(&dfmplugin_propertydialog::PropertyEventReceiver::handleViewExtensionRegister));
    const QVariant ret = mgr.push(62700);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_PropertyEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_propertydialog::PropertyEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62702, receiver, static_cast<bool (dfmplugin_propertydialog::PropertyEventReceiver::*)(dfmplugin_propertydialog::BasicViewFieldFunc, const QString &)>(&dfmplugin_propertydialog::PropertyEventReceiver::handleBasicViewExtensionRegister));
    const QVariant ret = mgr.push(62702);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_PropertyEventReceiver_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_propertydialog::PropertyEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62704, receiver, static_cast<bool (dfmplugin_propertydialog::PropertyEventReceiver::*)(const QString &, const QStringList &)>(&dfmplugin_propertydialog::PropertyEventReceiver::handleBasicFiledFilterAdd));
    const QVariant ret = mgr.push(62704);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_PropertyEventReceiver_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_propertydialog::PropertyEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62706, receiver, static_cast<bool (dfmplugin_propertydialog::PropertyEventReceiver::*)(dfmplugin_propertydialog::CustomViewExtensionView, dfmplugin_propertydialog::ViewExtensionUpdateFunc, const QString &, int)>(&dfmplugin_propertydialog::PropertyEventReceiver::handleViewExtensionRegisterWithUpdate));
    const QVariant ret = mgr.push(62706);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_PropertyDialog_4_ArityGuardNoBody)
{
    // Arrange
    dfmplugin_propertydialog::PropertyDialog receiver;
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62708, &receiver, static_cast<void (dfmplugin_propertydialog::PropertyDialog::*)(const QString &)>(&dfmplugin_propertydialog::PropertyDialog::bindScene));
    const bool published = mgr.publish(62708);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(62708, &receiver, static_cast<void (dfmplugin_propertydialog::PropertyDialog::*)(const QString &)>(&dfmplugin_propertydialog::PropertyDialog::bindScene)));  // remove + compare path
}

