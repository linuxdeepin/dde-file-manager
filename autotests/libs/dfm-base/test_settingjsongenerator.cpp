// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingjsongenerator.cpp
 * @brief Unit tests for SettingJsonGenerator pure-logic API.
 */

#include <gtest/gtest.h>

#include <dfm-base/settingdialog/settingjsongenerator.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace dfmbase;

class SettingJsonGeneratorTest : public testing::Test
{
protected:
    SettingJsonGenerator *gen { nullptr };

    void SetUp() override
    {
        gen = SettingJsonGenerator::instance();
    }
};

TEST_F(SettingJsonGeneratorTest, AddTopGroupSucceeds)
{
    EXPECT_TRUE(gen->addGroup("top1", "Top One"));
}

TEST_F(SettingJsonGeneratorTest, AddDuplicateTopGroupFails)
{
    EXPECT_TRUE(gen->addGroup("topdup", "Dup"));
    EXPECT_FALSE(gen->addGroup("topdup", "Dup Again"));
}

TEST_F(SettingJsonGeneratorTest, AddTooDeepGroupFails)
{
    EXPECT_FALSE(gen->addGroup("a.b.c", "Too Deep"));
}

TEST_F(SettingJsonGeneratorTest, AddGroupWithLeadingDotFails)
{
    EXPECT_FALSE(gen->addGroup(".bad", "Bad"));
}

TEST_F(SettingJsonGeneratorTest, AddGroupWithTrailingDotFails)
{
    EXPECT_FALSE(gen->addGroup("bad.", "Bad"));
}

TEST_F(SettingJsonGeneratorTest, AddSubGroupAutoCreatesTop)
{
    EXPECT_TRUE(gen->addGroup("top2.sub1", "Sub One"));
    EXPECT_TRUE(gen->hasGroup("top2.sub1"));
}

TEST_F(SettingJsonGeneratorTest, RemoveTopGroupSucceeds)
{
    gen->addGroup("toprm", "Rm");
    EXPECT_TRUE(gen->removeGroup("toprm"));
    EXPECT_FALSE(gen->hasGroup("toprm"));
}

TEST_F(SettingJsonGeneratorTest, RemoveNonExistentGroupFails)
{
    EXPECT_FALSE(gen->removeGroup("no.such.group"));
}

TEST_F(SettingJsonGeneratorTest, AddConfigWithWrongDepthFails)
{
    QVariantMap cfg;
    cfg["key"] = "item1";
    cfg["text"] = "Item";
    // only 1 dot (2 fragments) -> wrong depth, needs 2 dots
    EXPECT_FALSE(gen->addConfig("top3.sub2", cfg));
}

TEST_F(SettingJsonGeneratorTest, AddConfigKeyMismatchFails)
{
    QVariantMap cfg;
    cfg["key"] = "wrongkey";
    cfg["text"] = "Item";
    EXPECT_FALSE(gen->addConfig("topcfg.subcfg.itemX", cfg));
}

TEST_F(SettingJsonGeneratorTest, AddCheckBoxConfigSucceeds)
{
    EXPECT_TRUE(gen->addCheckBoxConfig("cbgrp.cbsub.cbitem", "Check Me", true));
    EXPECT_TRUE(gen->hasConfig("cbgrp.cbsub.cbitem"));
}

TEST_F(SettingJsonGeneratorTest, AddComboboxStringListConfigSucceeds)
{
    EXPECT_TRUE(gen->addComboboxConfig("combo.grp.item", "Choose",
                                       QStringList { "a", "b" }, 0));
}

TEST_F(SettingJsonGeneratorTest, AddComboboxVariantMapConfigSucceeds)
{
    QVariantMap opts;
    opts["a"] = 1;
    opts["b"] = 2;
    EXPECT_TRUE(gen->addComboboxConfig("combo2.grp2.item", "Choose", opts, QVariant(1)));
}

TEST_F(SettingJsonGeneratorTest, AddPathComboboxConfigSucceeds)
{
    QVariantMap opts;
    opts["p1"] = "/tmp";
    EXPECT_TRUE(gen->addPathComboboxConfig("pcmb.grp.item", "Path", opts, QVariant("/tmp")));
}

TEST_F(SettingJsonGeneratorTest, AddSliderConfigSimpleSucceeds)
{
    EXPECT_TRUE(gen->addSliderConfig("slider.grp.item", "Slider", 100, 0, 50));
}

TEST_F(SettingJsonGeneratorTest, AddSliderConfigWithIconsSucceeds)
{
    EXPECT_TRUE(gen->addSliderConfig("slider2.grp.item", "Slider", "left", "right", 100, 0, 50));
}

TEST_F(SettingJsonGeneratorTest, AddSliderConfigWithValuesSucceeds)
{
    EXPECT_TRUE(gen->addSliderConfig("slider3.grp.item", "Slider", "left", "right", 100, 0,
                                     QVariantList { 10, 20 }, 50));
}

TEST_F(SettingJsonGeneratorTest, RemoveConfigSucceeds)
{
    gen->addCheckBoxConfig("rmgrp.rmsub.rmitem", "Rm", false);
    EXPECT_TRUE(gen->removeConfig("rmgrp.rmsub.rmitem"));
    EXPECT_FALSE(gen->hasConfig("rmgrp.rmsub.rmitem"));
}

TEST_F(SettingJsonGeneratorTest, RemoveConfigWrongDepthFails)
{
    EXPECT_FALSE(gen->removeConfig("only.one.dot"));
}

TEST_F(SettingJsonGeneratorTest, GenSettingJsonProducesValidJson)
{
    gen->addGroup("jsontop", "JSON Top");
    gen->addCheckBoxConfig("jsontop.jsonsub.jsonitem", "JSON Item", true);
    QByteArray json = gen->genSettingJson();
    EXPECT_FALSE(json.isEmpty());
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    ASSERT_EQ(err.error, QJsonParseError::NoError) << err.errorString().toStdString();
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    ASSERT_TRUE(obj.contains("groups"));
    EXPECT_TRUE(obj.value("groups").isArray());
}

TEST_F(SettingJsonGeneratorTest, HasConfigNonExistentReturnsFalse)
{
    EXPECT_FALSE(gen->hasConfig("nonexistent.config.key"));
}
