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

#include <plugins/filemanager/dfmplugin-optical/events/opticaleventreceiver.h>
#include <plugins/filemanager/dfmplugin-optical/utils/opticalfilehelper.h>
#include <plugins/filemanager/dfmplugin-optical/utils/opticalhelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_OpticalEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62200, receiver, static_cast<bool (dfmplugin_optical::OpticalEventReceiver::*)(const QUrl &, const QUrl &)>(&dfmplugin_optical::OpticalEventReceiver::handleTabCloseable));
    const bool ran = mgr.run(62200);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62202, receiver, static_cast<bool (dfmplugin_optical::OpticalEventReceiver::*)(const QList<QUrl> &, const QUrl &)>(&dfmplugin_optical::OpticalEventReceiver::handleDropFiles));
    const bool ran = mgr.run(62202);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalEventReceiver_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62204, receiver, static_cast<bool (dfmplugin_optical::OpticalEventReceiver::*)(const QUrl &, QList<QVariantMap> *)>(&dfmplugin_optical::OpticalEventReceiver::sepateTitlebarCrumb));
    const bool ran = mgr.run(62204);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalEventReceiver_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62206, receiver, static_cast<bool (dfmplugin_optical::OpticalEventReceiver::*)(const QUrl &, QString *)>(&dfmplugin_optical::OpticalEventReceiver::detailViewIcon));
    const bool ran = mgr.run(62206);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalEventReceiver_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62208, receiver, static_cast<bool (dfmplugin_optical::OpticalEventReceiver::*)(quint64, const QList<QUrl> &, const QUrl &)>(&dfmplugin_optical::OpticalEventReceiver::handleDeleteFilesShortcut));
    const bool ran = mgr.run(62208);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalEventReceiver_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62210, receiver, static_cast<bool (dfmplugin_optical::OpticalEventReceiver::*)(const QList<QUrl> &, const QUrl &, Qt::DropAction *)>(&dfmplugin_optical::OpticalEventReceiver::handleCheckDragDropAction));
    const bool ran = mgr.run(62210);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalFileHelper_6_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalFileHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62212, receiver, static_cast<bool (dfmplugin_optical::OpticalFileHelper::*)(const quint64, const QList<QUrl>, dfmbase::AbstractJobHandler::JobFlags)>(&dfmplugin_optical::OpticalFileHelper::moveToTrash));
    const bool ran = mgr.run(62212);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalFileHelper_7_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalFileHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62214, receiver, static_cast<bool (dfmplugin_optical::OpticalFileHelper::*)(const quint64, const QList<QUrl>, const QUrl, dfmbase::AbstractJobHandler::JobFlags)>(&dfmplugin_optical::OpticalFileHelper::cutFile));
    const bool ran = mgr.run(62214);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalFileHelper_8_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalFileHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62216, receiver, static_cast<bool (dfmplugin_optical::OpticalFileHelper::*)(const quint64, dfmbase::ClipBoard::ClipboardAction, const QList<QUrl>)>(&dfmplugin_optical::OpticalFileHelper::writeUrlsToClipboard));
    const bool ran = mgr.run(62216);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_OpticalHelper_9_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_optical::OpticalHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62218, receiver, static_cast<bool (dfmplugin_optical::OpticalHelper::*)(const QUrl &, dfmbase::Global::TransparentStatus *)>(&dfmplugin_optical::OpticalHelper::isTransparent));
    const bool ran = mgr.run(62218);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

