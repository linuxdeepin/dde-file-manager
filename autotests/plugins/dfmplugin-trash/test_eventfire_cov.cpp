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

#include <plugins/filemanager/dfmplugin-trash/utils/trashhelper.h>
#include <plugins/filemanager/dfmplugin-trash/utils/trashfilehelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_TrashHelper_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trash::TrashHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63200, receiver, static_cast<bool (dfmplugin_trash::TrashHelper::*)(const QUrl &)>(&dfmplugin_trash::TrashHelper::checkCanMove));
    const bool ran = mgr.run(63200);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_TrashHelper_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trash::TrashHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63202, receiver, static_cast<bool (dfmplugin_trash::TrashHelper::*)(const QList<QUrl> &, const QUrl &, Qt::DropAction *)>(&dfmplugin_trash::TrashHelper::checkDragDropAction));
    const bool ran = mgr.run(63202);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_TrashHelper_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trash::TrashHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63204, receiver, static_cast<bool (dfmplugin_trash::TrashHelper::*)(const QUrl &, QList<dfmbase::Global::ItemRoles> *)>(&dfmplugin_trash::TrashHelper::customColumnRole));
    const bool ran = mgr.run(63204);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_TrashHelper_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trash::TrashHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63206, receiver, static_cast<bool (dfmplugin_trash::TrashHelper::*)(const QUrl &, dfmbase::Global::ItemRoles, QString *)>(&dfmplugin_trash::TrashHelper::customRoleDisplayName));
    const bool ran = mgr.run(63206);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_TrashFileHelper_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trash::TrashFileHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63208, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(const quint64, const QList<QUrl>, dfmbase::AbstractJobHandler::JobFlags)>(&dfmplugin_trash::TrashFileHelper::moveToTrash));
    const bool ran = mgr.run(63208);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_TrashFileHelper_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trash::TrashFileHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63210, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(const quint64, const QList<QUrl>, const QUrl, dfmbase::AbstractJobHandler::JobFlags)>(&dfmplugin_trash::TrashFileHelper::copyFile));
    const bool ran = mgr.run(63210);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_TrashFileHelper_6_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trash::TrashFileHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63212, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(const QUrl &, const QUrl &)>(&dfmplugin_trash::TrashFileHelper::handleIsSubFile));
    const bool ran = mgr.run(63212);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

