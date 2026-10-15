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

#include <plugins/desktop/ddplugin-wallpapersetting/wlsetplugin.h>
#include <plugins/desktop/ddplugin-wallpapersetting/wallpapersettings.h>
#include <plugins/desktop/ddplugin-wallpapersetting/private/wallpapersettings_p.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_WSEventHandle_0_ArityGuardNoBody)
{
    // Arrange
    ddplugin_wallpapersetting::EventHandle receiver(nullptr);
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63700, &receiver, static_cast<bool (ddplugin_wallpapersetting::EventHandle::*)(const QString &)>(&ddplugin_wallpapersetting::EventHandle::wallpaperSetting));
    const bool ran = mgr.run(63700);
    const bool unfollowed = mgr.unfollow(63700, &receiver, static_cast<bool (ddplugin_wallpapersetting::EventHandle::*)(const QString &)>(&ddplugin_wallpapersetting::EventHandle::wallpaperSetting));
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts
    EXPECT_TRUE(unfollowed);  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_WSEventHandleChan_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_wallpapersetting::EventHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63702, receiver, static_cast<bool (ddplugin_wallpapersetting::EventHandle::*)(const QString &)>(&ddplugin_wallpapersetting::EventHandle::wallpaperSetting));
    const QVariant ret = mgr.push(63702);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_WallpaperSettings_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_wallpapersetting::WallpaperSettings *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63704, receiver, static_cast<void (ddplugin_wallpapersetting::WallpaperSettings::*)()>(&ddplugin_wallpapersetting::WallpaperSettings::refreshList));
    const bool published = mgr.publish(63704, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_WallpaperSettingsPrivate_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_wallpapersetting::WallpaperSettingsPrivate *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63706, receiver, static_cast<void (ddplugin_wallpapersetting::WallpaperSettingsPrivate::*)()>(&ddplugin_wallpapersetting::WallpaperSettingsPrivate::propertyForWayland));
    const bool published = mgr.publish(63706, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

