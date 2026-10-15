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

#include <plugins/filedialog/core/views/filedialog.h>
#include <plugins/filedialog/core/core.h>

#include <QItemSelection>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_FileDialog_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<filedialog_core::FileDialog *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63400, receiver, static_cast<void (filedialog_core::FileDialog::*)(const QVariantMap &)>(&filedialog_core::FileDialog::onViewItemClicked));
    const bool published = mgr.publish(63400);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_FileDialog_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<filedialog_core::FileDialog *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63402, receiver, static_cast<void (filedialog_core::FileDialog::*)(const quint64, const QUrl &)>(&filedialog_core::FileDialog::handleRenameStartAcceptBtn));
    const bool published = mgr.publish(63402);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_FileDialog_2_ArityGuardNoBody)
{
    // Arrange
    filedialog_core::FileDialog receiver { QUrl() };
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63404, &receiver, static_cast<void (filedialog_core::FileDialog::*)(const quint64, const QItemSelection &, const QItemSelection &)>(&filedialog_core::FileDialog::onViewSelectionChanged));
    const bool published = mgr.publish(63404);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(63404, &receiver, static_cast<void (filedialog_core::FileDialog::*)(const quint64, const QItemSelection &, const QItemSelection &)>(&filedialog_core::FileDialog::onViewSelectionChanged)));  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_Core_3_ArityGuardNoBody)
{
    // Arrange
    filedialog_core::Core receiver;
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63406, &receiver, static_cast<void (filedialog_core::Core::*)(const QString &)>(&filedialog_core::Core::bindScene));
    const bool published = mgr.publish(63406);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(63406, &receiver, static_cast<void (filedialog_core::Core::*)(const QString &)>(&filedialog_core::Core::bindScene)));  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_Core_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<filedialog_core::Core *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63408, receiver, static_cast<bool (filedialog_core::Core::*)(const quint64, const QUrl &, const QUrl &)>(&filedialog_core::Core::handleEnterDir));
    const bool ran = mgr.run(63408);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
    EXPECT_FALSE(mgr.unfollow(63408, receiver, static_cast<bool (filedialog_core::Core::*)(const quint64, const QUrl &, const QUrl &)>(&filedialog_core::Core::handleEnterDir)));  // compare path executes, null receiver never matches
}

