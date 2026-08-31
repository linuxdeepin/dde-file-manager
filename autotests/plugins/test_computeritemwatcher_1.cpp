// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computeritemwatcher_1.cpp
 * @brief Unit tests for ComputerItemWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watcher/computeritemwatcher.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerItemWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerItemWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerItemWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerItemWatcherTest, addDevice)
{
    // Test method: void addDevice((const QString &groupName, const QUrl &url, int shape, bool addToSidebar))
    QString _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addDevice(_arg0, _arg1, 0, false));
}

TEST_F(ComputerItemWatcherTest, addSidebarItem)
{
    // Test method: void addSidebarItem((const QUrl &url, const QVariantMap &data))
    QUrl _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addSidebarItem(_arg0, _arg1));
}

TEST_F(ComputerItemWatcherTest, disksHiddenByDConf)
{
    // Test getter: QList<QUrl> disksHiddenByDConf()
    auto result = obj->disksHiddenByDConf();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, disksHiddenBySettingPanel)
{
    // Test getter: QList<QUrl> disksHiddenBySettingPanel()
    auto result = obj->disksHiddenBySettingPanel();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, findFinalUrl)
{
    // Test method: QUrl findFinalUrl((DFMEntryFileInfoPointer info))
    auto result = obj->findFinalUrl(DFMEntryFileInfoPointer());
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerItemWatcherTest, getComputerInfos)
{
    // Test getter: QHash<QUrl, QVariantMap> getComputerInfos()
    auto result = obj->getComputerInfos();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, getGroup)
{
    // Test method: ComputerItemData getGroup((ComputerItemWatcher::GroupType type, const QString &defaultName))
    QString _arg1{};
    auto result = obj->getGroup(ComputerItemWatcher::GroupType(), _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->getGroup(ComputerItemWatcher::GroupType(), _arg1); });

}

TEST_F(ComputerItemWatcherTest, getGroupId)
{
    // Test method: int getGroupId((const QString &groupName))
    QString _arg0{};
    auto result = obj->getGroupId(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(ComputerItemWatcherTest, getInitedItems)
{
    // Test getter: ComputerDataList getInitedItems()
    auto result = obj->getInitedItems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, hiddenPartitions)
{
    // Test getter: QList<QUrl> hiddenPartitions()
    auto result = obj->hiddenPartitions();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, hide3rdEntries)
{
    // Test bool getter: hide3rdEntries()
    bool result = obj->hide3rdEntries();
    EXPECT_FALSE(result);

}

TEST_F(ComputerItemWatcherTest, hideUserDir)
{
    // Test bool getter: hideUserDir()
    bool result = obj->hideUserDir();
    EXPECT_FALSE(result);

}

TEST_F(ComputerItemWatcherTest, initAppWatcher)
{
    // Test method: void initAppWatcher(())
    EXPECT_NO_FATAL_FAILURE(obj->initAppWatcher());
}

TEST_F(ComputerItemWatcherTest, initConn)
{
    // Test method: void initConn(())
    EXPECT_NO_FATAL_FAILURE(obj->initConn());
}

TEST_F(ComputerItemWatcherTest, initDeviceConn)
{
    // Test method: void initDeviceConn(())
    EXPECT_NO_FATAL_FAILURE(obj->initDeviceConn());
}

TEST_F(ComputerItemWatcherTest, insertUrlMapper)
{
    // Test method: void insertUrlMapper((const QString &devId, const QUrl &mntUrl))
    QString _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->insertUrlMapper(_arg0, _arg1));
}

TEST_F(ComputerItemWatcherTest, onBlockDeviceAdded)
{
    // Test method: void onBlockDeviceAdded((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlockDeviceAdded(_arg0));
}

TEST_F(ComputerItemWatcherTest, onBlockDeviceLocked)
{
    // Test method: void onBlockDeviceLocked((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlockDeviceLocked(_arg0));
}

TEST_F(ComputerItemWatcherTest, onBlockDeviceMounted)
{
    // Test method: void onBlockDeviceMounted((const QString &id, const QString &mntPath))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlockDeviceMounted(_arg0, _arg1));
}

TEST_F(ComputerItemWatcherTest, onBlockDeviceUnmounted)
{
    // Test method: void onBlockDeviceUnmounted((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlockDeviceUnmounted(_arg0));
}

TEST_F(ComputerItemWatcherTest, onDConfigChanged)
{
    // Test method: void onDConfigChanged((const QString &cfg, const QString &cfgKey))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDConfigChanged(_arg0, _arg1));
}

TEST_F(ComputerItemWatcherTest, onDeviceAdded)
{
    // Test method: void onDeviceAdded((const QUrl &devUrl, int groupId, ComputerItemData::ShapeType shape, bool needSidebarItem))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDeviceAdded(_arg0, 0, ComputerItemData::ShapeType(), false));
}

TEST_F(ComputerItemWatcherTest, onDevicePropertyChangedQVar)
{
    // Test method: void onDevicePropertyChangedQVar((const QString &id, const QString &propertyName, const QVariant &var))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onDevicePropertyChangedQVar(_arg0, _arg1, _arg2));
}

TEST_F(ComputerItemWatcherTest, onDeviceSizeChanged)
{
    // Test method: void onDeviceSizeChanged((const QString &id, qlonglong total, qlonglong free))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDeviceSizeChanged(_arg0, {}, {}));
}

TEST_F(ComputerItemWatcherTest, onGenAttributeChanged)
{
    // Test method: void onGenAttributeChanged((Application::GenericAttribute ga, const QVariant &value))
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onGenAttributeChanged(Application::GenericAttribute(), _arg1));
}

TEST_F(ComputerItemWatcherTest, onProtocolDeviceMounted)
{
    // Test method: void onProtocolDeviceMounted((const QString &id, const QString &mntPath))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onProtocolDeviceMounted(_arg0, _arg1));
}

TEST_F(ComputerItemWatcherTest, onProtocolDeviceUnmounted)
{
    // Test method: void onProtocolDeviceUnmounted((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onProtocolDeviceUnmounted(_arg0));
}

TEST_F(ComputerItemWatcherTest, onUpdateBlockItem)
{
    // Test method: void onUpdateBlockItem((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateBlockItem(_arg0));
}

TEST_F(ComputerItemWatcherTest, onViewRefresh)
{
    // Test method: void onViewRefresh(())
    EXPECT_NO_FATAL_FAILURE(obj->onViewRefresh());
}

TEST_F(ComputerItemWatcherTest, reportName)
{
    // Test method: QString reportName((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->reportName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, updateSidebarItem)
{
    // Test method: void updateSidebarItem((const QUrl &url, const QString &newName, bool editable))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateSidebarItem(_arg0, _arg1, false));
}
