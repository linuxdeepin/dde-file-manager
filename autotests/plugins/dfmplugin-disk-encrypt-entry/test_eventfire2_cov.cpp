// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventfire2_cov.cpp - explicit EventChannelManager::push specialization
// sweep for the DiskEncryptMenuCreator pointer payload pack.
//
// Production code pushes a DiskEncryptMenuCreator* through the slot channel;
// that exact template instantiation never runs under tests. The case below
// registers a null receiver on a fresh topic id (the framework's `if (s)`
// guard keeps the slot body from running) and then fires the same push pack
// both by numeric topic id and by space/topic name.

#include "menu/diskencryptmenuscene.h"
#include "plugin_diskencryptentry.h"

#include <QVariant>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFire2SweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFire2SweepTest, PushSend_DiskEncryptMenuCreator_0_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannelManager mgr;
    auto *receiver = static_cast<dfmplugin_diskenc::DiskEncryptEntry *>(nullptr);
    // Act (DiskEncryptMenuCreator*&& parameter: pass a prvalue pointer so it binds)
    const bool connected = mgr.connect(63850, receiver,
                                       static_cast<void (dfmplugin_diskenc::DiskEncryptEntry::*)(const QString &)>(&dfmplugin_diskenc::DiskEncryptEntry::onComputerMenuSceneAdded));
    const QVariant byId = mgr.push<QString, dfmplugin_diskenc::DiskEncryptMenuCreator *>(63850, QString::fromLatin1("scene"), static_cast<dfmplugin_diskenc::DiskEncryptMenuCreator *>(nullptr));
    const QVariant byName = mgr.push<QString, dfmplugin_diskenc::DiskEncryptMenuCreator *>("dfm.ut.diskenc", "slot_ut_menucreator", QString::fromLatin1("scene"), static_cast<dfmplugin_diskenc::DiskEncryptMenuCreator *>(nullptr));
    // Assert
    EXPECT_TRUE(connected);
    EXPECT_FALSE(byId.isValid());  // arity guard on void slot: 2 payload args vs 1-param receiver
    EXPECT_FALSE(byName.isValid());  // converter unregistered: name-based push never dispatches
}
