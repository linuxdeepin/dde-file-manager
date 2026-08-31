// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pluginmanager.cpp
 * @brief Unit tests for PluginManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/lifecycle/pluginmanager.h"

#include <QTest>

using namespace src;

class PluginManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PluginManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PluginManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PluginManagerTest, loadPlugin)
{
    // Test method: bool loadPlugin((PluginMetaObjectPointer &pointer))
    PluginMetaObjectPointer _arg0{};
    auto result = obj->loadPlugin(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, initPlugin)
{
    // Test method: bool initPlugin((PluginMetaObjectPointer &pointer))
    PluginMetaObjectPointer _arg0{};
    auto result = obj->initPlugin(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, startPlugin)
{
    // Test method: bool startPlugin((PluginMetaObjectPointer &pointer))
    PluginMetaObjectPointer _arg0{};
    auto result = obj->startPlugin(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, stopPlugin)
{
    // Test method: bool stopPlugin((PluginMetaObjectPointer &pointer))
    PluginMetaObjectPointer _arg0{};
    auto result = obj->stopPlugin(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, readPlugins)
{
    // Test bool getter: readPlugins()
    bool result = obj->readPlugins();
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, loadPlugins)
{
    // Test bool getter: loadPlugins()
    bool result = obj->loadPlugins();
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, initPlugins)
{
    // Test bool getter: initPlugins()
    bool result = obj->initPlugins();
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, startPlugins)
{
    // Test bool getter: startPlugins()
    bool result = obj->startPlugins();
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, stopPlugins)
{
    // Test method: void stopPlugins(())
    EXPECT_NO_FATAL_FAILURE(obj->stopPlugins());
}

TEST_F(PluginManagerTest, pluginPaths)
{
    // Test getter: QStringList pluginPaths()
    auto result = obj->pluginPaths();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginManagerTest, blackList)
{
    // Test getter: QStringList blackList()
    auto result = obj->blackList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginManagerTest, readQueue)
{
    // Test getter: QQueue<PluginMetaObjectPointer> readQueue()
    auto result = obj->readQueue();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginManagerTest, loadQueue)
{
    // Test getter: QQueue<PluginMetaObjectPointer> loadQueue()
    auto result = obj->loadQueue();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginManagerTest, addPluginIID)
{
    // Test method: void addPluginIID((const QString &pluginIID))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addPluginIID(_arg0));
}

TEST_F(PluginManagerTest, setPluginPaths)
{
    // Test setter: void setPluginPaths((const QStringList &pluginPaths))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPluginPaths(_arg0));
}

TEST_F(PluginManagerTest, pluginMetaObj)
{
    // Test method: PluginMetaObjectPointer pluginMetaObj((const QString &pluginName, const QString version))
    QString _arg0{};
    auto result = obj->pluginMetaObj(_arg0, QString());
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(PluginManagerTest, pluginIIDs)
{
    // Test getter: QStringList pluginIIDs()
    auto result = obj->pluginIIDs();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginManagerTest, lazyLoadList)
{
    // Test getter: QStringList lazyLoadList()
    auto result = obj->lazyLoadList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginManagerTest, addBlackPluginName)
{
    // Test method: void addBlackPluginName((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addBlackPluginName(_arg0));
}

TEST_F(PluginManagerTest, setLazyLoadFilter)
{
    // Test setter: void setLazyLoadFilter((std::function<bool(const QString &)> filter))
    std::function<bool( QString )> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setLazyLoadFilter(_arg0));
}

TEST_F(PluginManagerTest, setBlackListFilter)
{
    // Test setter: void setBlackListFilter((std::function<bool(const QString &)> filter))
    std::function<bool( QString )> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setBlackListFilter(_arg0));
}

TEST_F(PluginManagerTest, isAllPluginsInitialized)
{
    // Test bool getter: isAllPluginsInitialized()
    bool result = obj->isAllPluginsInitialized();
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, isAllPluginsStarted)
{
    // Test bool getter: isAllPluginsStarted()
    bool result = obj->isAllPluginsStarted();
    EXPECT_FALSE(result);

}

TEST_F(PluginManagerTest, addLazyLoadPluginName)
{
    // Test method: void addLazyLoadPluginName((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addLazyLoadPluginName(_arg0));
}

TEST_F(PluginManagerTest, setQtVersionInsensitivePluginNames)
{
    // Test setter: void setQtVersionInsensitivePluginNames((const QStringList &names))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setQtVersionInsensitivePluginNames(_arg0));
}

TEST_F(PluginManagerTest, PluginManagerPrivate)
{
    // Test method:  PluginManagerPrivate(())
    EXPECT_NO_FATAL_FAILURE(obj->PluginManagerPrivate());
}

TEST_F(PluginManagerTest, friend)
{
    // Test getter: Q_OBJECT friend()
    EXPECT_NO_FATAL_FAILURE({ obj->friend(); });
}

TEST_F(PluginManagerTest, d)
{
    // Test getter: QSharedPointer<PluginManagerPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
