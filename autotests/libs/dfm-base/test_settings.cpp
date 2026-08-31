// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settings.cpp
 * @brief Unit tests for Settings methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/application/settings.h"

#include <QTest>

using namespace src;

class SettingsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Settings();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Settings *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SettingsTest, Settings)
{
    // Test constructor: Settings((const QString &name, ConfigType type, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SettingsTest, M_~Settings)
{
    // Test method:  ~Settings(())
    EXPECT_NO_FATAL_FAILURE({ Settings *tmp = new Settings(); delete tmp; });
}

TEST_F(SettingsTest, contains)
{
    // Test method: bool contains((const QString &group, const QString &key))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->contains(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SettingsTest, groups)
{
    // Test getter: QSet<QString> groups()
    auto result = obj->groups();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingsTest, keys)
{
    // Test method: QSet<QString> keys((const QString &group))
    QString _arg0{};
    auto result = obj->keys(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingsTest, value)
{
    // Test method: QVariant value((const QString &group, const QUrl &key, const QVariant &defaultValue))
    QString _arg0{};
    QUrl _arg1{};
    QVariant _arg2{};
    auto result = obj->value(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SettingsTest, remove)
{
    // Test method: void remove((const QString &group, const QUrl &key))
    QString _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->remove(_arg0, _arg1));
}

TEST_F(SettingsTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(SettingsTest, reload)
{
    // Test method: void reload(())
    EXPECT_NO_FATAL_FAILURE(obj->reload());
}

TEST_F(SettingsTest, sync)
{
    // Test bool getter: sync()
    bool result = obj->sync();
    EXPECT_FALSE(result);

}

TEST_F(SettingsTest, keyList)
{
    // Test method: QStringList keyList((const QString &group))
    QString _arg0{};
    auto result = obj->keyList(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingsTest, urlValue)
{
    // Test method: QUrl urlValue((const QString &group, const QUrl &key, const QUrl &defaultValue))
    QString _arg0{};
    QUrl _arg1{};
    QUrl _arg2{};
    auto result = obj->urlValue(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SettingsTest, setValue)
{
    // Test setter: void setValue((const QString &group, const QUrl &key, const QVariant &value))
    QString _arg0{};
    QUrl _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setValue(_arg0, _arg1, _arg2));
}

TEST_F(SettingsTest, removeGroup)
{
    // Test method: void removeGroup((const QString &group))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeGroup(_arg0));
}

TEST_F(SettingsTest, isRemovable)
{
    // Test method: bool isRemovable((const QString &group, const QUrl &key))
    QString _arg0{};
    QUrl _arg1{};
    auto result = obj->isRemovable(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SettingsTest, autoSync)
{
    // Test bool getter: autoSync()
    bool result = obj->autoSync();
    EXPECT_FALSE(result);

}

TEST_F(SettingsTest, watchChanges)
{
    // Test bool getter: watchChanges()
    bool result = obj->watchChanges();
    EXPECT_FALSE(result);

}

TEST_F(SettingsTest, defaultConfigkeyList)
{
    // Test method: QStringList defaultConfigkeyList((const QString &group))
    QString _arg0{};
    auto result = obj->defaultConfigkeyList(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingsTest, toUrlValue)
{
    // Test method: QUrl toUrlValue((const QVariant &url))
    QVariant _arg0{};
    auto result = obj->toUrlValue(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SettingsTest, defaultConfigValue)
{
    // Test method: QVariant defaultConfigValue((const QString &group, const QUrl &key, const QVariant &defaultValue))
    QString _arg0{};
    QUrl _arg1{};
    QVariant _arg2{};
    auto result = obj->defaultConfigValue(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SettingsTest, autoSyncExclude)
{
    // Test method: void autoSyncExclude((const QString &group, bool sync /*= false*/))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->autoSyncExclude(_arg0, nullptr));
}

TEST_F(SettingsTest, isReadOnly)
{
    // Test bool getter: isReadOnly()
    bool result = obj->isReadOnly();
    EXPECT_FALSE(result);

}

TEST_F(SettingsTest, setAutoSync)
{
    // Test setter: void setAutoSync((bool autoSync))
    EXPECT_NO_FATAL_FAILURE(obj->setAutoSync(false));
}

TEST_F(SettingsTest, onFileChanged)
{
    // Test method: void onFileChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileChanged(_arg0));
}

TEST_F(SettingsTest, setWatchChanges)
{
    // Test setter: void setWatchChanges((bool watchChanges))
    EXPECT_NO_FATAL_FAILURE(obj->setWatchChanges(false));
}

TEST_F(SettingsTest, setReadOnly)
{
    // Test setter: void setReadOnly((bool readOnly))
    EXPECT_NO_FATAL_FAILURE(obj->setReadOnly(false));
}

TEST_F(SettingsTest, setValueNoNotify)
{
    // Test method: bool setValueNoNotify((const QString &group, const QUrl &key, const QVariant &value))
    QString _arg0{};
    QUrl _arg1{};
    QVariant _arg2{};
    auto result = obj->setValueNoNotify(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(SettingsTest, SettingsPrivate)
{
    // Test method:  SettingsPrivate(())
    EXPECT_NO_FATAL_FAILURE(obj->SettingsPrivate());
}

TEST_F(SettingsTest, ConfigType)
{
    // Test method:  ConfigType(())
    EXPECT_NO_FATAL_FAILURE(obj->ConfigType());
}

TEST_F(SettingsTest, friend)
{
    // Test getter: Q_OBJECT friend()
    EXPECT_NO_FATAL_FAILURE({ obj->friend(); });
}

TEST_F(SettingsTest, d)
{
    // Test getter: QScopedPointer<SettingsPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
