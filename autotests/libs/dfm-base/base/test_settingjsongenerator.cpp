// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>

#include "stubext.h"

#include <dfm-base/settingdialog/settingjsongenerator.h>

using namespace dfmbase;

class TestSettingJsonGenerator : public testing::Test
{
public:
    void SetUp() override
    {
        generator = SettingJsonGenerator::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    /*
     * Helper function to parse JSON and verify structure
     */
    QJsonObject parseJson(const QByteArray &json)
    {
        QJsonDocument doc = QJsonDocument::fromJson(json);
        EXPECT_FALSE(doc.isNull());
        EXPECT_TRUE(doc.isObject());
        return doc.object();
    }

    stub_ext::StubExt stub;
    SettingJsonGenerator *generator { nullptr };
};

// ========== Basic Functionality Tests ==========

TEST_F(TestSettingJsonGenerator, instance_ReturnsSingleton)
{
    // Test that instance() returns the same singleton
    auto instance1 = SettingJsonGenerator::instance();
    auto instance2 = SettingJsonGenerator::instance();

    ASSERT_TRUE(instance1 != nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestSettingJsonGenerator, genSettingJson_EmptyGenerator)
{
    // Test generating JSON from empty generator
    QByteArray json = generator->genSettingJson();

    EXPECT_FALSE(json.isEmpty());

    QJsonObject root = parseJson(json);
    EXPECT_TRUE(root.contains("groups"));
}

// ========== Group Management Tests ==========

TEST_F(TestSettingJsonGenerator, addGroup_TopLevelGroup)
{
    // Test adding a top-level group
    QString key = "test_top_group";
    QString name = "Test Top Group";

    bool result = generator->addGroup(key, name);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasGroup(key));
}

TEST_F(TestSettingJsonGenerator, addGroup_SecondLevelGroup)
{
    // Test adding a second-level group
    QString topKey = "top_group";
    QString topName = "Top Group";
    QString configKey = "top_group.config_group";
    QString configName = "Config Group";

    generator->addGroup(topKey, topName);
    bool result = generator->addGroup(configKey, configName);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasGroup(configKey));
}

TEST_F(TestSettingJsonGenerator, addGroup_ThirdLevelGroup_Fails)
{
    // Test that adding a third-level group fails
    QString key = "level1.level2.level3";
    QString name = "Invalid Group";

    bool result = generator->addGroup(key, name);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addGroup_KeyStartsWithDot_Fails)
{
    // Test that key starting with dot fails
    QString key = ".invalid_key";
    QString name = "Invalid";

    bool result = generator->addGroup(key, name);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addGroup_KeyEndsWithDot_Fails)
{
    // Test that key ending with dot fails
    QString key = "invalid_key.";
    QString name = "Invalid";

    bool result = generator->addGroup(key, name);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addGroup_DuplicateTopGroup_Fails)
{
    // Test that adding duplicate top group fails
    QString key = "duplicate_top";
    QString name1 = "Name 1";
    QString name2 = "Name 2";

    generator->addGroup(key, name1);
    bool result = generator->addGroup(key, name2);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addGroup_DuplicateConfigGroup_Fails)
{
    // Test that adding duplicate config group fails
    QString topKey = "top";
    QString configKey = "top.duplicate_config";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Name 1");
    bool result = generator->addGroup(configKey, "Name 2");

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addGroup_ConfigGroupWithoutTopGroup)
{
    // Test adding config group without top group (should create temp top group)
    QString configKey = "nonexistent_top.config";
    QString configName = "Config";

    bool result = generator->addGroup(configKey, configName);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasGroup(configKey));
}

TEST_F(TestSettingJsonGenerator, removeGroup_TopLevelGroup)
{
    // Test removing a top-level group
    QString key = "removable_top";
    QString name = "Removable Top";

    generator->addGroup(key, name);
    EXPECT_TRUE(generator->hasGroup(key));

    bool result = generator->removeGroup(key);

    EXPECT_TRUE(result);
    EXPECT_FALSE(generator->hasGroup(key));
}

TEST_F(TestSettingJsonGenerator, removeGroup_ConfigGroup)
{
    // Test removing a config group
    QString topKey = "top";
    QString configKey = "top.removable_config";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    bool result = generator->removeGroup(configKey);

    EXPECT_TRUE(result);
    EXPECT_FALSE(generator->hasGroup(configKey));
}

TEST_F(TestSettingJsonGenerator, removeGroup_NonExistentGroup_Fails)
{
    // Test removing a non-existent group
    QString key = "nonexistent";

    bool result = generator->removeGroup(key);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, removeGroup_InvalidKey_Fails)
{
    // Test removing with invalid key (too many levels)
    QString key = "level1.level2.level3";

    bool result = generator->removeGroup(key);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, hasGroup_ExistingGroup)
{
    // Test hasGroup with existing group
    QString key = "existing_group";
    generator->addGroup(key, "Existing");

    EXPECT_TRUE(generator->hasGroup(key));
}

TEST_F(TestSettingJsonGenerator, hasGroup_NonExistingGroup)
{
    // Test hasGroup with non-existing group
    EXPECT_FALSE(generator->hasGroup("nonexistent_group"));
}

// ========== Config Management Tests ==========

TEST_F(TestSettingJsonGenerator, addConfig_ValidConfig)
{
    // Test adding a valid config
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.item";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QVariantMap config {
        { "key", "item" },
        { "text", "Test Item" },
        { "type", "checkbox" },
        { "default", true }
    };

    bool result = generator->addConfig(itemKey, config);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, addConfig_InvalidLevel_Fails)
{
    // Test that config with wrong level fails
    QString key = "top.item";   // Only 2 levels instead of 3

    QVariantMap config {
        { "key", "item" },
        { "text", "Invalid" }
    };

    bool result = generator->addConfig(key, config);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addConfig_KeyStartsWithDot_Fails)
{
    // Test config with key starting with dot
    QString key = ".top.config.item";

    QVariantMap config {
        { "key", "item" }
    };

    bool result = generator->addConfig(key, config);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addConfig_KeyEndsWithDot_Fails)
{
    // Test config with key ending with dot
    QString key = "top.config.item.";

    QVariantMap config {
        { "key", "item." }
    };

    bool result = generator->addConfig(key, config);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addConfig_DoubleDot_Fails)
{
    // Test config with double dot
    QString key = "top..item";

    QVariantMap config {
        { "key", "item" }
    };

    bool result = generator->addConfig(key, config);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addConfig_MismatchedKey_Fails)
{
    // Test config with mismatched key in map
    QString key = "top.config.item1";

    QVariantMap config {
        { "key", "item2" },   // Mismatch!
        { "text", "Test" }
    };

    bool result = generator->addConfig(key, config);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addConfig_DuplicateConfig_Fails)
{
    // Test adding duplicate config
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.duplicate";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QVariantMap config1 {
        { "key", "duplicate" },
        { "text", "First" }
    };

    QVariantMap config2 {
        { "key", "duplicate" },
        { "text", "Second" }
    };

    generator->addConfig(itemKey, config1);
    bool result = generator->addConfig(itemKey, config2);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, removeConfig_ValidConfig)
{
    // Test removing a valid config
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.removable";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QVariantMap config {
        { "key", "removable" },
        { "text", "Removable" }
    };

    generator->addConfig(itemKey, config);
    EXPECT_TRUE(generator->hasConfig(itemKey));

    bool result = generator->removeConfig(itemKey);

    EXPECT_TRUE(result);
    EXPECT_FALSE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, removeConfig_InvalidLevel_Fails)
{
    // Test removing config with invalid level
    QString key = "top.item";

    bool result = generator->removeConfig(key);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, removeConfig_DoubleDot_Fails)
{
    // Test removing config with double dot
    QString key = "top..item";

    bool result = generator->removeConfig(key);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, hasConfig_ExistingConfig)
{
    // Test hasConfig with existing config
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.existing";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QVariantMap config {
        { "key", "existing" },
        { "text", "Existing" }
    };

    generator->addConfig(itemKey, config);

    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, hasConfig_NonExistingConfig)
{
    // Test hasConfig with non-existing config
    EXPECT_FALSE(generator->hasConfig("top.config.nonexistent"));
}

// ========== Specialized Config Addition Tests ==========

TEST_F(TestSettingJsonGenerator, addCheckBoxConfig_Valid)
{
    // Test adding a checkbox config
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.checkbox_item";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    bool result = generator->addCheckBoxConfig(itemKey, "Checkbox Text", false);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, addCheckBoxConfig_DefaultTrue)
{
    // Test adding checkbox with default true value
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.checkbox_default";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    bool result = generator->addCheckBoxConfig(itemKey, "Checkbox");

    EXPECT_TRUE(result);
}

TEST_F(TestSettingJsonGenerator, addComboboxConfig_WithStringList)
{
    // Test adding combobox with string list
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.combo_item";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QStringList options { "Option 1", "Option 2", "Option 3" };
    bool result = generator->addComboboxConfig(itemKey, "Combobox", options, 1);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, addComboboxConfig_WithVariantMap)
{
    // Test adding combobox with variant map
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.combo_variant";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QVariantMap options {
        { "values", QStringList { "Val1", "Val2" } },
        { "keys", QStringList { "key1", "key2" } }
    };

    bool result = generator->addComboboxConfig(itemKey, "Combobox", options, QVariant("key1"));

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, addPathComboboxConfig_Valid)
{
    // Test adding path combobox config
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.path_combo";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QVariantMap options {
        { "values", QStringList { "Path1", "Path2" } },
        { "keys", QStringList { "/path1", "/path2" } }
    };

    bool result = generator->addPathComboboxConfig(itemKey, "Path Combobox", options, QVariant("/path1"));

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, addSliderConfig_Basic)
{
    // Test adding basic slider config
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.slider_basic";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    bool result = generator->addSliderConfig(itemKey, "Slider", 10, 0, 5);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, addSliderConfig_WithIcons)
{
    // Test adding slider with icons
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.slider_icons";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    bool result = generator->addSliderConfig(itemKey, "Slider", "left_icon", "right_icon", 100, 0, 50);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

TEST_F(TestSettingJsonGenerator, addSliderConfig_WithValueList)
{
    // Test adding slider with value list
    QString topKey = "top";
    QString configKey = "top.config";
    QString itemKey = "top.config.slider_values";

    generator->addGroup(topKey, "Top");
    generator->addGroup(configKey, "Config");

    QVariantList values { 10, 20, 30, 40, 50 };
    bool result = generator->addSliderConfig(itemKey, "Slider", "left", "right", 4, 0, values, 2);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

// ========== JSON Generation Tests ==========

TEST_F(TestSettingJsonGenerator, genSettingJson_WithTopGroup)
{
    // Test JSON generation with top group
    generator->addGroup("test_top", "Test Top Group");

    QByteArray json = generator->genSettingJson();
    QJsonObject root = parseJson(json);

    ASSERT_TRUE(root.contains("groups"));
    QJsonArray groups = root["groups"].toArray();
    EXPECT_GT(groups.size(), 0);
}

TEST_F(TestSettingJsonGenerator, genSettingJson_WithConfigGroup)
{
    // Test JSON generation with config group
    generator->addGroup("top", "Top");
    generator->addGroup("top.config", "Config");

    QByteArray json = generator->genSettingJson();
    QJsonObject root = parseJson(json);

    ASSERT_TRUE(root.contains("groups"));
    QJsonArray topGroups = root["groups"].toArray();
    EXPECT_GT(topGroups.size(), 0);
}

TEST_F(TestSettingJsonGenerator, genSettingJson_WithCompleteStructure)
{
    // Test JSON generation with complete structure
    generator->addGroup("top", "Top Group");
    generator->addGroup("top.config", "Config Group");
    generator->addCheckBoxConfig("top.config.checkbox", "Checkbox", true);
    generator->addComboboxConfig("top.config.combo", "Combo", QStringList { "A", "B" }, 0);

    QByteArray json = generator->genSettingJson();
    QJsonObject root = parseJson(json);

    ASSERT_TRUE(root.contains("groups"));
    QJsonArray topGroups = root["groups"].toArray();
    EXPECT_GT(topGroups.size(), 0);

    // Verify the structure contains expected fields
    EXPECT_FALSE(json.isEmpty());
}

TEST_F(TestSettingJsonGenerator, genSettingJson_MultipleTopGroups)
{
    // Test JSON with multiple top groups
    generator->addGroup("top1", "Top Group 1");
    generator->addGroup("top2", "Top Group 2");
    generator->addGroup("top1.config", "Config 1");
    generator->addGroup("top2.config", "Config 2");

    QByteArray json = generator->genSettingJson();
    QJsonObject root = parseJson(json);

    ASSERT_TRUE(root.contains("groups"));
    QJsonArray topGroups = root["groups"].toArray();
    EXPECT_GE(topGroups.size(), 2);
}

// ========== Edge Cases and Error Handling ==========

TEST_F(TestSettingJsonGenerator, addGroup_EmptyKey_Fails)
{
    // Test adding group with empty key
    bool result = generator->addGroup("", "Empty Key");

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, addConfig_EmptyKey_Fails)
{
    // Test adding config with empty key
    QVariantMap config {
        { "key", "" },
        { "text", "Empty" }
    };

    bool result = generator->addConfig("", config);

    EXPECT_FALSE(result);
}

TEST_F(TestSettingJsonGenerator, genSettingJson_ValidJsonFormat)
{
    // Test that generated JSON is valid
    generator->addGroup("valid", "Valid Group");
    generator->addGroup("valid.config", "Config");
    generator->addCheckBoxConfig("valid.config.item", "Item", false);

    QByteArray json = generator->genSettingJson();

    QJsonDocument doc = QJsonDocument::fromJson(json);
    EXPECT_FALSE(doc.isNull());
    EXPECT_TRUE(doc.isObject());
}

TEST_F(TestSettingJsonGenerator, addConfig_WithoutParentGroups)
{
    // Test adding config creates temporary parent groups
    QString itemKey = "auto_top.auto_config.auto_item";

    QVariantMap config {
        { "key", "auto_item" },
        { "text", "Auto Item" }
    };

    bool result = generator->addConfig(itemKey, config);

    EXPECT_TRUE(result);
    EXPECT_TRUE(generator->hasConfig(itemKey));
}

// ========== Integration Tests ==========

TEST_F(TestSettingJsonGenerator, Integration_CompleteWorkflow)
{
    // Test complete workflow: add, check, generate, remove
    QString topKey = "workflow_top";
    QString configKey = "workflow_top.workflow_config";
    QString itemKey = "workflow_top.workflow_config.workflow_item";

    // Add groups
    EXPECT_TRUE(generator->addGroup(topKey, "Workflow Top"));
    EXPECT_TRUE(generator->addGroup(configKey, "Workflow Config"));

    // Add config
    EXPECT_TRUE(generator->addCheckBoxConfig(itemKey, "Workflow Item", true));

    // Verify existence
    EXPECT_TRUE(generator->hasGroup(topKey));
    EXPECT_TRUE(generator->hasGroup(configKey));
    EXPECT_TRUE(generator->hasConfig(itemKey));

    // Generate JSON
    QByteArray json = generator->genSettingJson();
    EXPECT_FALSE(json.isEmpty());

    // Remove config
    EXPECT_TRUE(generator->removeConfig(itemKey));
    EXPECT_FALSE(generator->hasConfig(itemKey));

    // Remove groups
    EXPECT_TRUE(generator->removeGroup(configKey));
    EXPECT_FALSE(generator->hasGroup(configKey));
}

TEST_F(TestSettingJsonGenerator, Integration_MultipleConfigTypes)
{
    // Test adding multiple config types
    QString topKey = "multi_top";
    QString configKey = "multi_top.multi_config";

    generator->addGroup(topKey, "Multi Top");
    generator->addGroup(configKey, "Multi Config");

    // Add various config types
    EXPECT_TRUE(generator->addCheckBoxConfig("multi_top.multi_config.checkbox", "Checkbox", false));
    EXPECT_TRUE(generator->addComboboxConfig("multi_top.multi_config.combo", "Combo", QStringList { "A", "B", "C" }, 1));
    EXPECT_TRUE(generator->addSliderConfig("multi_top.multi_config.slider", "Slider", 10, 0, 5));

    // Generate and verify JSON
    QByteArray json = generator->genSettingJson();
    QJsonObject root = parseJson(json);

    EXPECT_TRUE(root.contains("groups"));
}
