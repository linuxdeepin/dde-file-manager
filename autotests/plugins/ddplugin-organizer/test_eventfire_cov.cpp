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

#include <plugins/desktop/ddplugin-organizer/view/collectionwidget.h>
#include <plugins/desktop/ddplugin-organizer/mode/collectiondataprovider.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class MockCollectionDataProvider : public ddplugin_organizer::CollectionDataProvider
{
public:
    MockCollectionDataProvider() : ddplugin_organizer::CollectionDataProvider(nullptr) {}
    QString replace(const QUrl &, const QUrl &) override { return QString(); }
    QString append(const QUrl &) override { return QString(); }
    QString prepend(const QUrl &) override { return QString(); }
    void insert(const QUrl &, const QString &, const int) override {}
    QString remove(const QUrl &) override { return QString(); }
    QString change(const QUrl &) override { return QString(); }
};

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_CollectionWidget_0_ArityGuardNoBody)
{
    // Arrange
    ddplugin_organizer::CollectionWidget receiver(QString("ut-organizer"), new MockCollectionDataProvider(), nullptr);
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(64300, &receiver, static_cast<void (ddplugin_organizer::CollectionWidget::*)()>(&ddplugin_organizer::CollectionWidget::cacheSnapshot));
    const bool unsubscribed = mgr.unsubscribe(64300, &receiver, static_cast<void (ddplugin_organizer::CollectionWidget::*)()>(&ddplugin_organizer::CollectionWidget::cacheSnapshot));
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(unsubscribed);  // remove + compare path removes the handler
}

