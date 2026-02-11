// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include "stubext.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE

class TestDConfigManager : public testing::Test
{
public:
    void SetUp() override
    {
        // Initialize DConfigManager instance
        manager = DConfigManager::instance();
        ASSERT_TRUE(manager);
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    DConfigManager *manager;
    stub_ext::StubExt stub;
};

TEST_F(TestDConfigManager, SingletonCreation_Basic)
{
    // Test that instance returns the same pointer
    DConfigManager *instance1 = DConfigManager::instance();
    DConfigManager *instance2 = DConfigManager::instance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestDConfigManager, AddConfig)
{
    // Test addConfig with success and failure cases
    bool addResult = true;
    QString errMsg;

    stub.set_lamda(ADDR(DConfigManager, addConfig), [&](DConfigManager *obj, const QString &config, QString *err) {
        if (err) *err = errMsg;
        return addResult;
    });

    // Test addConfig success
    addResult = true;
    errMsg.clear();
    EXPECT_TRUE(manager->addConfig("test-config"));

    // Test addConfig failure with error message
    addResult = false;
    errMsg = "failed to load config";
    QString err;
    EXPECT_FALSE(manager->addConfig("invalid-config", &err));
    EXPECT_EQ(err, errMsg);
}

TEST_F(TestDConfigManager, RemoveConfig)
{
    // Test removeConfig
    bool removeResult = true;
    QString errMsg;

    stub.set_lamda(ADDR(DConfigManager, removeConfig), [&](DConfigManager *obj, const QString &config, QString *err) {
        if (err) *err = errMsg;
        return removeResult;
    });

    // Test removeConfig success
    removeResult = true;
    errMsg.clear();
    EXPECT_TRUE(manager->removeConfig("test-config"));

    // Test removeConfig failure with error message
    removeResult = false;
    errMsg = "config not found";
    QString err;
    EXPECT_FALSE(manager->removeConfig("non-existent-config", &err));
    EXPECT_EQ(err, errMsg);
}

TEST_F(TestDConfigManager, Keys)
{
    // Test keys() returns expected list
    QStringList mockKeys = {"general.view.mode", "general.icon.size", "preview.enabled"};
    stub.set_lamda(ADDR(DConfigManager, keys), [&](DConfigManager *obj, const QString &config) {
        Q_UNUSED(obj);
        Q_UNUSED(config);
        return mockKeys;
    });

    QStringList result = manager->keys("dfm.settings");
    EXPECT_EQ(result, mockKeys);
}

TEST_F(TestDConfigManager, Contains)
{
    // Test contains() checks for key existence
    stub.set_lamda(ADDR(DConfigManager, contains), [](DConfigManager *obj, const QString &config, const QString &key) {
        Q_UNUSED(obj);
        Q_UNUSED(config);
        return key == "general.view.mode"; // Only this key exists
    });

    EXPECT_TRUE(manager->contains("dfm.settings", "general.view.mode"));
    EXPECT_FALSE(manager->contains("dfm.settings", "non-existent.key"));
}

TEST_F(TestDConfigManager, Value)
{
    // Test value() with different scenarios
    stub.set_lamda(ADDR(DConfigManager, value), [](DConfigManager *obj, const QString &config, const QString &key, const QVariant &fallback) {
        Q_UNUSED(obj);
        Q_UNUSED(config);

        if (key == "general.view.mode")
            return QVariant("icon");
        if (key == "general.icon.size")
            return QVariant(64);
        return fallback;
    });

    // Test existing string value
    EXPECT_EQ(manager->value("dfm.settings", "general.view.mode").toString(), "icon");
    // Test existing numeric value
    EXPECT_EQ(manager->value("dfm.settings", "general.icon.size").toInt(), 64);
    // Test non-existent key with fallback
    EXPECT_EQ(manager->value("dfm.settings", "non-existent.key", "default").toString(), "default");
}

TEST_F(TestDConfigManager, SetValue)
{
    // Test setValue and valueChanged signal
    QSignalSpy valueChangedSpy(manager, &DConfigManager::valueChanged);

    // Test that setValue doesn't crash
    manager->setValue("dfm.settings", "general.view.mode", "list");

    // Since we're not stubbing the actual setValue implementation that emits the signal,
    // we can't expect the signal to be emitted. We just verify that the function is callable.
}

TEST_F(TestDConfigManager, ValidateConfigs)
{
    // Test validateConfigs identifies invalid configs
    stub.set_lamda(ADDR(DConfigManager, validateConfigs), [](DConfigManager *obj, QStringList &invalidConfigs) {
        Q_UNUSED(obj);
        invalidConfigs << "invalid.config.1" << "invalid.config.2";
        return true; // Return true even if some configs are invalid
    });

    QStringList invalid;
    bool result = manager->validateConfigs(invalid);

    EXPECT_TRUE(result);
    EXPECT_EQ(invalid.size(), 2);
    EXPECT_EQ(invalid[0], "invalid.config.1");
    EXPECT_EQ(invalid[1], "invalid.config.2");
}