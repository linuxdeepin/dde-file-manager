// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingjsongenerator.cpp
 * @brief Unit tests for SettingJsonGenerator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/configs/settingjsongenerator.h"

#include <QTest>

using namespace src;

class SettingJsonGeneratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SettingJsonGenerator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SettingJsonGenerator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SettingJsonGeneratorTest, SettingJsonGenerator)
{
    // Test constructor: SettingJsonGenerator(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(SettingJsonGeneratorTest, instance)
{
    // Test getter: SettingJsonGenerator instance()
    auto result = obj->instance();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, hasGroup)
{
    // Test method: bool hasGroup((const QString &key))
    QString _arg0{};
    auto result = obj->hasGroup(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, hasConfig)
{
    // Test method: bool hasConfig((const QString &key))
    QString _arg0{};
    auto result = obj->hasConfig(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, addGroup)
{
    // Test method: bool addGroup((const QString &key, const QString &name))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->addGroup(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, removeGroup)
{
    // Test method: bool removeGroup((const QString &key))
    QString _arg0{};
    auto result = obj->removeGroup(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, addConfig)
{
    // Test method: bool addConfig((const QString &key, const QVariantMap &config))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->addConfig(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, removeConfig)
{
    // Test method: bool removeConfig((const QString &key))
    QString _arg0{};
    auto result = obj->removeConfig(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, mergeGroups)
{
    // Test method: void mergeGroups(())
    EXPECT_NO_FATAL_FAILURE(obj->mergeGroups());
}

TEST_F(SettingJsonGeneratorTest, constructConfig)
{
    // Test method: QJsonObject constructConfig((const QString &key))
    QString _arg0{};
    auto result = obj->constructConfig(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, genSettingJson)
{
    // Test getter: QByteArray genSettingJson()
    auto result = obj->genSettingJson();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, addComboboxConfig)
{
    // Test method: bool addComboboxConfig((const QString &key, const QString &name, const QVariantMap &options, QVariant defaultVal))
    QString _arg0{};
    QString _arg1{};
    QVariantMap _arg2{};
    auto result = obj->addComboboxConfig(_arg0, _arg1, _arg2, QVariant());
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, addSliderConfig)
{
    // Test method: bool addSliderConfig((const QString &key,
                                           const QString &name,
                                           const QString &leftIcon,
                                           const QString &rightIcon,
                                           int maxVal,
                                           int minVal,
                                           QVariantList valueList,
                                           int defaultVal))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    QString _arg3{};
    auto result = obj->addSliderConfig(_arg0, _arg1, _arg2, _arg3, 0, 0, QVariantList(), 0);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, constructTopGroup)
{
    // Test method: QJsonObject constructTopGroup((const QString &key))
    QString _arg0{};
    auto result = obj->constructTopGroup(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, constructConfigGroup)
{
    // Test method: QJsonObject constructConfigGroup((const QString &key))
    QString _arg0{};
    auto result = obj->constructConfigGroup(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, addCheckBoxConfig)
{
    // Test method: bool addCheckBoxConfig((const QString &key, const QString &text, bool defaultVal))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->addCheckBoxConfig(_arg0, _arg1, false);
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, addPathComboboxConfig)
{
    // Test method: bool addPathComboboxConfig((const QString &key, const QString &name, const QVariantMap &options, QVariant defaultVal))
    QString _arg0{};
    QString _arg1{};
    QVariantMap _arg2{};
    auto result = obj->addPathComboboxConfig(_arg0, _arg1, _arg2, QVariant());
    EXPECT_FALSE(result);

}

TEST_F(SettingJsonGeneratorTest, configs)
{
    // Test getter: QMap<QString, QVariantMap> configs()
    auto result = obj->configs();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, topGroups)
{
    // Test getter: QMap<QString, QString> topGroups()
    auto result = obj->topGroups();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, configGroups)
{
    // Test getter: QMap<QString, QString> configGroups()
    auto result = obj->configGroups();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, tmpTopGroups)
{
    // Test getter: QMap<QString, QString> tmpTopGroups()
    auto result = obj->tmpTopGroups();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingJsonGeneratorTest, tmpConfigGroups)
{
    // Test getter: QMap<QString, QString> tmpConfigGroups()
    auto result = obj->tmpConfigGroups();
    EXPECT_TRUE(result.isEmpty());

}
