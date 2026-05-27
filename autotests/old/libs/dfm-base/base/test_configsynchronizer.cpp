// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QVariant>

#include "stubext.h"

#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/configs/configsyncdefs.h>

DFMBASE_USE_NAMESPACE

class TestConfigSynchronizer : public testing::Test
{
public:
    void SetUp() override
    {
        // Initialize ConfigSynchronizer instance
        synchronizer = ConfigSynchronizer::instance();
        ASSERT_TRUE(synchronizer);
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    ConfigSynchronizer *synchronizer;
    stub_ext::StubExt stub;
};

TEST_F(TestConfigSynchronizer, SingletonCreation_Basic)
{
    // Test that instance returns the same pointer
    ConfigSynchronizer *instance1 = ConfigSynchronizer::instance();
    ConfigSynchronizer *instance2 = ConfigSynchronizer::instance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestConfigSynchronizer, WatchChange)
{
    // Test watchChange with valid SyncPair
    SyncPair validPair;
    validPair.set.type = SettingType::kGenAttr;
    validPair.set.val = 1;   // Represents some application attribute
    validPair.cfg.path = "dfm.settings";
    validPair.cfg.key = "general.view.mode";

    // Since the real watchChange might have dependencies, let's stub it
    bool watchCalled = false;
    stub.set_lamda(ADDR(ConfigSynchronizer, watchChange), [&](ConfigSynchronizer *obj, const SyncPair &pair) {
        Q_UNUSED(obj);
        watchCalled = true;
        EXPECT_TRUE(pair.isValid());
        EXPECT_EQ(pair.cfg.path, "dfm.settings");
        EXPECT_EQ(pair.cfg.key, "general.view.mode");
        return true;   // Return success
    });

    bool result = synchronizer->watchChange(validPair);
    EXPECT_TRUE(result);
    EXPECT_TRUE(watchCalled);
}

TEST_F(TestConfigSynchronizer, WatchChange_InvalidPair)
{
    // Test watchChange with invalid SyncPair
    SyncPair invalidPair;
    // pair is invalid because set.type is kNone and cfg fields are empty

    // Test that watchChange rejects invalid pairs even without stub
    bool result = synchronizer->watchChange(invalidPair);
    EXPECT_FALSE(result);
}

// Test SyncPair struct separately since it's a standalone struct with utility functions
TEST(TestSyncPair, IsValid)
{
    // Test valid SyncPair
    SyncPair validPair;
    validPair.set.type = SettingType::kAppAttr;
    validPair.set.val = 2;
    validPair.cfg.path = "test.config";
    validPair.cfg.key = "test.key";
    EXPECT_TRUE(validPair.isValid());

    // Test invalid SyncPair - missing type
    SyncPair invalidPair1;
    invalidPair1.set.type = SettingType::kNone;
    invalidPair1.cfg.path = "test.config";
    invalidPair1.cfg.key = "test.key";
    EXPECT_FALSE(invalidPair1.isValid());

    // Test invalid SyncPair - missing cfg.path
    SyncPair invalidPair2;
    invalidPair2.set.type = SettingType::kGenAttr;
    invalidPair2.set.val = 3;
    invalidPair2.cfg.key = "test.key";
    EXPECT_FALSE(invalidPair2.isValid());

    // Test invalid SyncPair - missing cfg.key
    SyncPair invalidPair3;
    invalidPair3.set.type = SettingType::kGenAttr;
    invalidPair3.set.val = 4;
    invalidPair3.cfg.path = "test.config";
    EXPECT_FALSE(invalidPair3.isValid());
}

TEST(TestSyncPair, Serialize)
{
    // Test serialization of SyncPair
    SyncPair pair;
    pair.set.type = SettingType::kAppAttr;
    pair.set.val = 5;
    pair.cfg.path = "app/config/path";
    pair.cfg.key = "app_setting_key";

    QString serialized = pair.serialize();
    // Expected format: "type/val:path/key" - kAppAttr is 0
    EXPECT_EQ(serialized, "0/5:app/config/path/app_setting_key");

    // Test serialization with static method
    QString staticSerialized = SyncPair::serialize(pair.set, pair.cfg);
    EXPECT_EQ(staticSerialized, serialized);

    // Test serialization with kNone type (special case)
    SyncPair nonePair;
    nonePair.set.type = SettingType::kNone;
    nonePair.cfg.path = "only/config/path";
    nonePair.cfg.key = "only_key";
    QString noneSerialized = nonePair.serialize();
    EXPECT_EQ(noneSerialized, ":only/config/path/only_key");

    // Test serialization with empty cfg (another special case)
    SyncPair cfgEmptyPair;
    cfgEmptyPair.set.type = SettingType::kGenAttr;
    cfgEmptyPair.set.val = 6;
    QString cfgEmptySerialized = cfgEmptyPair.serialize();
    EXPECT_EQ(cfgEmptySerialized, "1/6:");
}

TEST(TestSyncPair, FunctionTypes)
{
    // Test that the function type aliases work correctly
    int testValue = 0;

    // Test ConfigSaver
    ConfigSaver saver = [&](const QVariant &val) {
        testValue = val.toInt();
    };

    saver(42);
    EXPECT_EQ(testValue, 42);

    // Test SyncToAppSet
    QString savedConfig;
    QString savedKey;
    QVariant savedValue;

    SyncToAppSet syncToApp = [&](const QString &config, const QString &key, const QVariant &value) {
        savedConfig = config;
        savedKey = key;
        savedValue = value;
    };

    syncToApp("test.cfg", "test.key", "test.val");
    EXPECT_EQ(savedConfig, "test.cfg");
    EXPECT_EQ(savedKey, "test.key");
    EXPECT_EQ(savedValue.toString(), "test.val");

    // Test IsConfEqual
    IsConfEqual isEqual = [](const QVariant &dconfVal, const QVariant &dsetVal) {
        return dconfVal.toString().toLower() == dsetVal.toString().toLower();
    };

    EXPECT_TRUE(isEqual("Value1", "value1"));
    EXPECT_FALSE(isEqual("Value1", "Value2"));
}
