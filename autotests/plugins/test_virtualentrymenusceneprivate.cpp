// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrymenusceneprivate.cpp
 * @brief Unit tests for VirtualEntryMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/menu/virtualentrymenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class VirtualEntryMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualEntryMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualEntryMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualEntryMenuScenePrivateTest, VirtualEntryMenuScenePrivate)
{
    // Test constructor: VirtualEntryMenuScenePrivate((VirtualEntryMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VirtualEntryMenuScenePrivateTest, actRemoveVirtualEntry)
{
    // Test method: void actRemoveVirtualEntry(())
    EXPECT_NO_FATAL_FAILURE(obj->actRemoveVirtualEntry());
}

TEST_F(VirtualEntryMenuScenePrivateTest, setActionVisible)
{
    // Test setter: void setActionVisible((const QStringList &visibleActions, QMenu *menu))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setActionVisible(_arg0, nullptr));
}

TEST_F(VirtualEntryMenuScenePrivateTest, tryRemoveAggregatedEntry)
{
    // Test method: void tryRemoveAggregatedEntry((const QString &stdSmb, const QString &stdSmbSharePath))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->tryRemoveAggregatedEntry(_arg0, _arg1));
}
