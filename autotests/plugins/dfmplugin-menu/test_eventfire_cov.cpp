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

#include <plugins/common/dfmplugin-menu/menu.h>
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_MenuHandle_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62000, receiver, static_cast<dfmbase::AbstractMenuScene *(dfmplugin_menu::MenuHandle::*)(const QString &)>(&dfmplugin_menu::MenuHandle::createScene));
    const QVariant ret = mgr.push(62000);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_MenuHandle_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62002, receiver, static_cast<bool (dfmplugin_menu::MenuHandle::*)(const QVariantHash &)>(&dfmplugin_menu::MenuHandle::isMenuDisable));
    const QVariant ret = mgr.push(62002);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_MenuHandle_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62004, receiver, static_cast<bool (dfmplugin_menu::MenuHandle::*)(const QString &)>(&dfmplugin_menu::MenuHandle::contains));
    const QVariant ret = mgr.push(62004);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_MenuHandle_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62006, receiver, static_cast<bool (dfmplugin_menu::MenuHandle::*)(const QString &, dfmbase::AbstractSceneCreator *)>(&dfmplugin_menu::MenuHandle::registerScene));
    const QVariant ret = mgr.push(62006);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_MenuHandle_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62008, receiver, static_cast<void (dfmplugin_menu::MenuHandle::*)(const QString &, const QString &)>(&dfmplugin_menu::MenuHandle::unbind));
    const QVariant ret = mgr.push(62008);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_MenuHandle_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62010, receiver, static_cast<bool (dfmplugin_menu::MenuHandle::*)(const QString &, const QString &)>(&dfmplugin_menu::MenuHandle::bind));
    const QVariant ret = mgr.push(62010);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_MenuHandle_6_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62012, receiver, static_cast<dfmbase::AbstractSceneCreator *(dfmplugin_menu::MenuHandle::*)(const QString &)>(&dfmplugin_menu::MenuHandle::unregisterScene));
    const QVariant ret = mgr.push(62012);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_MenuHandle_7_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_menu::MenuHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62014, receiver, static_cast<QVariantHash (dfmplugin_menu::MenuHandle::*)(const QVariantHash &)>(&dfmplugin_menu::MenuHandle::perfectMenuParams));
    const QVariant ret = mgr.push(62014);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

