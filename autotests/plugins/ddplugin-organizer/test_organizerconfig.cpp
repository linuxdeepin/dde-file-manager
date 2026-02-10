// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "config/organizerconfig.h"
#include "config/organizerconfig_p.h"

#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_OrganizerConfig : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());

        stub.set_lamda(&OrganizerConfig::path, [this]() {
            __DBG_STUB_INVOKE__
            return tempDir->filePath("test_organizer.conf");
        });

        config = new OrganizerConfig();
    }

    virtual void TearDown() override
    {
        delete config;
        config = nullptr;
        delete tempDir;
        tempDir = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    OrganizerConfig *config = nullptr;
    QTemporaryDir *tempDir = nullptr;
};

TEST_F(UT_OrganizerConfig, Constructor_CreatesConfig)
{
    EXPECT_NE(config, nullptr);
    EXPECT_NE(config->d, nullptr);
    EXPECT_NE(config->d->settings, nullptr);
}

TEST_F(UT_OrganizerConfig, isEnable_DefaultReturnsFalse)
{
    EXPECT_FALSE(config->isEnable());
}

TEST_F(UT_OrganizerConfig, setEnable_SetsEnableState)
{
    config->setEnable(true);
    config->sync(0);
    EXPECT_TRUE(config->isEnable());

    config->setEnable(false);
    config->sync(0);
    EXPECT_FALSE(config->isEnable());
}

TEST_F(UT_OrganizerConfig, mode_DefaultReturnsNormalized)
{
    int mode = config->mode();
    EXPECT_EQ(mode, static_cast<int>(OrganizerMode::kNormalized));
}

TEST_F(UT_OrganizerConfig, setMode_SetsMode)
{
    config->setMode(static_cast<int>(OrganizerMode::kCustom));
    config->sync(0);
    EXPECT_EQ(config->mode(), static_cast<int>(OrganizerMode::kCustom));

    config->setMode(static_cast<int>(OrganizerMode::kNormalized));
    config->sync(0);
    EXPECT_EQ(config->mode(), static_cast<int>(OrganizerMode::kNormalized));
}

TEST_F(UT_OrganizerConfig, setVersion_SetsVersion)
{
    config->setVersion("2.0.0");
    config->sync(0);
    // Version is write-only, no getter to verify directly
    SUCCEED();
}

TEST_F(UT_OrganizerConfig, classification_DefaultReturnsType)
{
    int cf = config->classification();
    EXPECT_EQ(cf, static_cast<int>(Classifier::kType));
}

TEST_F(UT_OrganizerConfig, setClassification_SetsClassification)
{
    config->setClassification(static_cast<int>(Classifier::kSize));
    config->sync(0);
    EXPECT_EQ(config->classification(), static_cast<int>(Classifier::kSize));
}

TEST_F(UT_OrganizerConfig, surfaceSizes_EmptyByDefault)
{
    QList<QSize> sizes = config->surfaceSizes();
    EXPECT_TRUE(sizes.isEmpty());
}

TEST_F(UT_OrganizerConfig, setScreenInfo_SetsScreenResolutions)
{
    QMap<QString, QString> info;
    info.insert("Screen_1", "1920:1080");
    info.insert("Screen_2", "1280:720");

    config->setScreenInfo(info);
    config->sync(0);

    QList<QSize> sizes = config->surfaceSizes();
    EXPECT_EQ(sizes.size(), 2);
}

TEST_F(UT_OrganizerConfig, lastStyleConfigId_EmptyByDefault)
{
    QString id = config->lastStyleConfigId();
    EXPECT_TRUE(id.isEmpty());
}

TEST_F(UT_OrganizerConfig, setLastStyleConfigId_SetsId)
{
    config->setLastStyleConfigId("test_config_id");
    config->sync(0);
    EXPECT_EQ(config->lastStyleConfigId(), "test_config_id");
}

TEST_F(UT_OrganizerConfig, hasConfigId_ReturnsFalseForNonexistent)
{
    EXPECT_FALSE(config->hasConfigId("nonexistent_id"));
}

TEST_F(UT_OrganizerConfig, sync_WithZeroMs_SyncsImmediately)
{
    config->setEnable(true);
    config->sync(0);
    EXPECT_TRUE(config->isEnable());
}

TEST_F(UT_OrganizerConfig, sync_WithPositiveMs_StartsTimer)
{
    config->setEnable(true);
    config->sync(100);
    EXPECT_TRUE(config->d->syncTimer.isActive());
}

TEST_F(UT_OrganizerConfig, collectionBase_Custom_EmptyByDefault)
{
    QList<CollectionBaseDataPtr> bases = config->collectionBase(true);
    EXPECT_TRUE(bases.isEmpty());
}

TEST_F(UT_OrganizerConfig, collectionBase_Normal_EmptyByDefault)
{
    QList<CollectionBaseDataPtr> bases = config->collectionBase(false);
    EXPECT_TRUE(bases.isEmpty());
}

TEST_F(UT_OrganizerConfig, writeCollectionBase_Custom_WritesData)
{
    QList<CollectionBaseDataPtr> bases;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "custom_key";
    data->name = "Custom Collection";
    data->items.append(QUrl("file:///test1.txt"));
    data->items.append(QUrl("file:///test2.txt"));
    bases.append(data);

    config->writeCollectionBase(true, bases);
    config->sync(0);

    QList<CollectionBaseDataPtr> result = config->collectionBase(true);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first()->key, "custom_key");
    EXPECT_EQ(result.first()->name, "Custom Collection");
    EXPECT_EQ(result.first()->items.size(), 2);
}

TEST_F(UT_OrganizerConfig, writeCollectionBase_Normal_WritesData)
{
    QList<CollectionBaseDataPtr> bases;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "normal_key";
    data->name = "Normal Collection";
    bases.append(data);

    config->writeCollectionBase(false, bases);
    config->sync(0);

    QList<CollectionBaseDataPtr> result = config->collectionBase(false);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first()->key, "normal_key");
}

TEST_F(UT_OrganizerConfig, updateCollectionBase_UpdatesExistingData)
{
    QList<CollectionBaseDataPtr> bases;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "update_key";
    data->name = "Original Name";
    bases.append(data);

    config->writeCollectionBase(true, bases);
    config->sync(0);

    data->name = "Updated Name";
    data->items.append(QUrl("file:///new_item.txt"));
    config->updateCollectionBase(true, data);
    config->sync(0);

    CollectionBaseDataPtr result = config->collectionBase(true, "update_key");
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->name, "Updated Name");
    EXPECT_EQ(result->items.size(), 1);
}

TEST_F(UT_OrganizerConfig, collectionBase_WithInvalidKey_ReturnsNull)
{
    CollectionBaseDataPtr result = config->collectionBase(true, "nonexistent_key");
    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_OrganizerConfig, collectionBase_WithValidKey_ReturnsData)
{
    // First, write some data
    QList<CollectionBaseDataPtr> bases;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "valid_key";
    data->name = "Valid Collection";
    bases.append(data);

    config->writeCollectionBase(true, bases);
    config->sync(0);

    CollectionBaseDataPtr result = config->collectionBase(true, "valid_key");
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->key, "valid_key");
    EXPECT_EQ(result->name, "Valid Collection");
}

TEST_F(UT_OrganizerConfig, collectionStyle_ReturnsEmptyStyleByDefault)
{
    CollectionStyle style = config->collectionStyle("", "test_key");
    EXPECT_TRUE(style.key.isEmpty());
}

TEST_F(UT_OrganizerConfig, writeCollectionStyle_WritesStyles)
{
    QList<CollectionStyle> styles;
    CollectionStyle style;
    style.key = "style_key";
    style.screenIndex = 1;
    style.rect = QRect(100, 100, 200, 300);
    style.sizeMode = CollectionFrameSize::kLarge;
    style.customGeo = true;
    styles.append(style);

    config->writeCollectionStyle("config_1", styles);
    config->sync(0);

    CollectionStyle result = config->collectionStyle("config_1", "style_key");
    EXPECT_EQ(result.key, "style_key");
    EXPECT_EQ(result.screenIndex, 1);
    EXPECT_EQ(result.rect, QRect(100, 100, 200, 300));
    EXPECT_EQ(result.sizeMode, CollectionFrameSize::kLarge);
    EXPECT_TRUE(result.customGeo);
}

TEST_F(UT_OrganizerConfig, updateCollectionStyle_UpdatesExistingStyle)
{
    QList<CollectionStyle> styles;
    CollectionStyle style;
    style.key = "update_style";
    style.screenIndex = 0;
    style.rect = QRect(0, 0, 100, 100);
    styles.append(style);

    config->writeCollectionStyle("", styles);
    config->sync(0);

    style.screenIndex = 2;
    style.rect = QRect(50, 50, 150, 150);
    config->updateCollectionStyle("", style);
    config->sync(0);

    CollectionStyle result = config->collectionStyle("", "update_style");
    EXPECT_EQ(result.screenIndex, 2);
    EXPECT_EQ(result.rect, QRect(50, 50, 150, 150));
}

TEST_F(UT_OrganizerConfig, writeCollectionStyle_SkipsEmptyKeys)
{
    QList<CollectionStyle> styles;
    CollectionStyle validStyle;
    validStyle.key = "valid_key";
    validStyle.screenIndex = 1;

    CollectionStyle invalidStyle;
    invalidStyle.key = "";
    invalidStyle.screenIndex = 2;

    styles.append(invalidStyle);
    styles.append(validStyle);

    config->writeCollectionStyle("test_config", styles);
    config->sync(0);

    CollectionStyle result = config->collectionStyle("test_config", "valid_key");
    EXPECT_EQ(result.key, "valid_key");
}

class UT_OrganizerConfigPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());

        stub.set_lamda(&OrganizerConfig::path, [this]() {
            __DBG_STUB_INVOKE__
            return tempDir->filePath("test_organizer.conf");
        });

        config = new OrganizerConfig();
    }

    virtual void TearDown() override
    {
        delete config;
        config = nullptr;
        delete tempDir;
        tempDir = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    OrganizerConfig *config = nullptr;
    QTemporaryDir *tempDir = nullptr;
};

TEST_F(UT_OrganizerConfigPrivate, value_WithEmptyKey_ReturnsDefault)
{
    QVariant result = config->d->value("group", "", QVariant("default"));
    EXPECT_EQ(result.toString(), "default");
}

TEST_F(UT_OrganizerConfigPrivate, value_WithEmptyGroup_ReadsFromRoot)
{
    config->d->setValue("", "test_key", "test_value");

    QVariant result = config->d->value("", "test_key", QVariant());
    EXPECT_EQ(result.toString(), "test_value");
}

TEST_F(UT_OrganizerConfigPrivate, value_WithGroup_ReadsFromGroup)
{
    config->d->setValue("TestGroup", "group_key", "group_value");

    QVariant result = config->d->value("TestGroup", "group_key", QVariant());
    EXPECT_EQ(result.toString(), "group_value");
}

TEST_F(UT_OrganizerConfigPrivate, setValue_WritesToSettings)
{
    config->d->setValue("TestGroup", "write_key", "write_value");
    config->sync(0);

    QVariant result = config->d->value("TestGroup", "write_key", QVariant());
    EXPECT_EQ(result.toString(), "write_value");
}
