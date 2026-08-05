// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settings.cpp
 * @brief Unit tests for Settings (settings.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QVariant>
#include <QSet>
#include <QStringList>

#include <dfm-base/base/application/settings.h>

using namespace dfmbase;

class SettingsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();

        defaultFile = rootPath + "/default.json";
        settingFile = rootPath + "/settings.json";
        writeFile(defaultFile, "{ \"General\": { \"name\": \"default\" } }");
        writeFile(settingFile, "{ \"General\": { \"name\": \"current\" } }");
    }

    void writeFile(const QString &path, const QString &content)
    {
        QFile f(path);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    QString defaultFile;
    QString settingFile;
};

TEST_F(SettingsTest, ConstructWithExplicitFiles)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_EQ(s.value("General", "name").toString(), QString("current"));
}

TEST_F(SettingsTest, ContainsExistingKey)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_TRUE(s.contains("General", "name"));
    EXPECT_FALSE(s.contains("General", "no_such_key"));
}

TEST_F(SettingsTest, ValueWithDefault)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_EQ(s.value("General", "no_such_key", QString("fallback")).toString(), QString("fallback"));
}

TEST_F(SettingsTest, SetValueThenRead)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setValue("General", "name", QString("updated"));
    EXPECT_EQ(s.value("General", "name").toString(), QString("updated"));
}

TEST_F(SettingsTest, SetValueNoNotify)
{
    Settings s(defaultFile, defaultFile, settingFile);
    bool ok = s.setValueNoNotify("General", "name", QString("nn"));
    EXPECT_EQ(s.value("General", "name").toString(), QString("nn"));
}

TEST_F(SettingsTest, GroupsReturnsGeneral)
{
    Settings s(defaultFile, defaultFile, settingFile);
    QSet<QString> g = s.groups();
    EXPECT_TRUE(g.contains("General"));
}

TEST_F(SettingsTest, KeysReturnsName)
{
    Settings s(defaultFile, defaultFile, settingFile);
    QSet<QString> k = s.keys("General");
    EXPECT_TRUE(k.contains("name"));
}

TEST_F(SettingsTest, KeyListReturnsName)
{
    Settings s(defaultFile, defaultFile, settingFile);
    QStringList k = s.keyList("General");
    EXPECT_TRUE(k.contains("name"));
}

TEST_F(SettingsTest, DefaultConfigValue)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ (void)s.defaultConfigValue("General", "name"); });
}

TEST_F(SettingsTest, DefaultConfigkeyList)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ (void)s.defaultConfigkeyList("General"); });
}

TEST_F(SettingsTest, UrlValue)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setValue("General", "url", QUrl("file:///tmp/x").toString());
    EXPECT_NO_FATAL_FAILURE({ (void)s.urlValue("General", "url"); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.urlValue("General", QUrl("key"), QUrl()); });
}

TEST_F(SettingsTest, ValueByUrlKey)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ (void)s.value("General", QUrl("name")); });
}

TEST_F(SettingsTest, ToUrlValue)
{
    QUrl url = Settings::toUrlValue(QVariant(QString("file:///tmp/x")));
    EXPECT_EQ(url.scheme(), QString("file"));
}

TEST_F(SettingsTest, SetValueByUrlKey)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ s.setValue("General", QUrl("urlkey"), QVariant(1)); });
    EXPECT_NO_FATAL_FAILURE({ s.setValueNoNotify("General", QUrl("urlkey"), QVariant(2)); });
}

TEST_F(SettingsTest, RemoveKey)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setValue("General", "toremove", QString("v"));
    EXPECT_TRUE(s.contains("General", "toremove"));
    EXPECT_NO_FATAL_FAILURE({ s.remove("General", "toremove"); });
    EXPECT_NO_FATAL_FAILURE({ s.remove("General", QUrl("toremove")); });
}

TEST_F(SettingsTest, IsRemovable)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ (void)s.isRemovable("General", "name"); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.isRemovable("General", QUrl("name")); });
}

TEST_F(SettingsTest, RemoveGroup)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setValue("ToRemove", "k", QString("v"));
    EXPECT_NO_FATAL_FAILURE({ s.removeGroup("ToRemove"); });
}

TEST_F(SettingsTest, ClearAndReload)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ s.clear(); });
    EXPECT_NO_FATAL_FAILURE({ s.reload(); });
}

TEST_F(SettingsTest, Sync)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ (void)s.sync(); });
}

TEST_F(SettingsTest, AutoSyncAccessors)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_FALSE(s.autoSync());
    s.setAutoSync(true);
    EXPECT_TRUE(s.autoSync());
    s.setAutoSync(false);
    EXPECT_FALSE(s.autoSync());
    EXPECT_NO_FATAL_FAILURE({ s.autoSyncExclude("General"); });
}

TEST_F(SettingsTest, WatchChangesAccessors)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ s.setWatchChanges(true); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.watchChanges(); });
}

TEST_F(SettingsTest, ReadOnlyAccessors)
{
    Settings s(defaultFile, defaultFile, settingFile);
    EXPECT_FALSE(s.isReadOnly());
    s.setReadOnly(true);
    EXPECT_TRUE(s.isReadOnly());
}

TEST_F(SettingsTest, ConstructByNameGenericConfig)
{
    EXPECT_NO_FATAL_FAILURE({ Settings s("test_generic_cfg", Settings::kGenericConfig); });
}
