// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocoldevicedisplaymanager_1.cpp
 * @brief Unit tests for ProtocolDeviceDisplayManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/protocoldevicedisplaymanager.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class ProtocolDeviceDisplayManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProtocolDeviceDisplayManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProtocolDeviceDisplayManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProtocolDeviceDisplayManagerTest, ProtocolDeviceDisplayManager)
{
    // Test constructor: ProtocolDeviceDisplayManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ProtocolDeviceDisplayManagerTest, displayMode)
{
    // Test getter: SmbDisplayMode displayMode()
    auto result = obj->displayMode();
    EXPECT_NO_FATAL_FAILURE({ obj->displayMode(); });

}

TEST_F(ProtocolDeviceDisplayManagerTest, handleItemRenamed)
{
    // Test method: void handleItemRenamed((const QUrl &entryUrl, const QString &name))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleItemRenamed(_arg0, _arg1));
}

TEST_F(ProtocolDeviceDisplayManagerTest, hookItemInsert)
{
    // Test method: bool hookItemInsert((const QUrl &entryUrl))
    QUrl _arg0{};
    auto result = obj->hookItemInsert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ProtocolDeviceDisplayManagerTest, hookItemsFilter)
{
    // Test method: bool hookItemsFilter((QList<QUrl> *entryUrls))
    auto result = obj->hookItemsFilter(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ProtocolDeviceDisplayManagerTest, isShowOfflineItem)
{
    // Test bool getter: isShowOfflineItem()
    bool result = obj->isShowOfflineItem();
    EXPECT_FALSE(result);

}

TEST_F(ProtocolDeviceDisplayManagerTest, onDConfigChanged)
{
    // Test method: void onDConfigChanged((const QString &g, const QString &k))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDConfigChanged(_arg0, _arg1));
}

TEST_F(ProtocolDeviceDisplayManagerTest, onDevMounted)
{
    // Test method: void onDevMounted((const QString &id, const QString &))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDevMounted(_arg0, _arg1));
}

TEST_F(ProtocolDeviceDisplayManagerTest, onJsonConfigChanged)
{
    // Test method: void onJsonConfigChanged((const QString &g, const QString &k, const QVariant &v))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onJsonConfigChanged(_arg0, _arg1, _arg2));
}

TEST_F(ProtocolDeviceDisplayManagerTest, onMenuSceneAdded)
{
    // Test method: void onMenuSceneAdded((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onMenuSceneAdded(_arg0));
}

TEST_F(ProtocolDeviceDisplayManagerTest, ProtocolDeviceDisplayManager_Destructor)
{
    // Test method:  ~ProtocolDeviceDisplayManager(())
    EXPECT_NO_FATAL_FAILURE({ ProtocolDeviceDisplayManager *tmp = new ProtocolDeviceDisplayManager(); delete tmp; });
}
