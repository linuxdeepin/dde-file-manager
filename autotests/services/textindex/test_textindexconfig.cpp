// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QSignalSpy>

#include "utils/textindexconfig.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_TextIndexConfig : public testing::Test
{
protected:
    void SetUp() override
    {
        // Mock DConfigManager to avoid dependency on actual configuration files
        stub.set_lamda(ADDR(DFMBASE_NAMESPACE::DConfigManager, addConfig), [](DFMBASE_NAMESPACE::DConfigManager *, const QString &, QString *) {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(DFMBASE_NAMESPACE::DConfigManager, value), [this](DFMBASE_NAMESPACE::DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) {
            __DBG_STUB_INVOKE__
            // Return test values based on key
            if (key == "autoIndexUpdateInterval") {
                return QVariant(300);
            } else if (key == "maxIndexFileSizeMB") {
                return QVariant(100);
            } else if (key == "supportedFileExtensions") {
                return QVariant::fromValue(QStringList({"txt", "pdf", "doc"}));
            }
            return defaultValue;
        });

        stub.set_lamda(ADDR(DFMBASE_NAMESPACE::DConfigManager, instance), []() -> DFMBASE_NAMESPACE::DConfigManager* {
            __DBG_STUB_INVOKE__
            static DFMBASE_NAMESPACE::DConfigManager mockManager;
            return &mockManager;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_TextIndexConfig, Instance_Singleton_ReturnsSameInstance)
{
    // Test singleton pattern
    TextIndexConfig &instance1 = TextIndexConfig::instance();
    TextIndexConfig &instance2 = TextIndexConfig::instance();
    
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(UT_TextIndexConfig, AutoIndexUpdateInterval_ValidValue_ReturnsExpectedValue)
{
    TextIndexConfig &config = TextIndexConfig::instance();
    
    int result = config.autoIndexUpdateInterval();
    
    EXPECT_GT(result, 0);  // Should be a positive value
}

TEST_F(UT_TextIndexConfig, MaxIndexFileSizeMB_ValidValue_ReturnsExpectedValue)
{
    TextIndexConfig &config = TextIndexConfig::instance();
    
    int result = config.maxIndexFileSizeMB();
    
    EXPECT_GT(result, 0);  // Should be a positive value
}

TEST_F(UT_TextIndexConfig, SupportedFileExtensions_ValidList_ReturnsNonEmptyList)
{
    TextIndexConfig &config = TextIndexConfig::instance();
    
    QStringList result = config.supportedFileExtensions();
    
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(UT_TextIndexConfig, ReloadConfig_Called_EmitsConfigChangedSignal)
{
    TextIndexConfig &config = TextIndexConfig::instance();
    QSignalSpy spy(&config, &TextIndexConfig::configChanged);
    
    config.reloadConfig();
    
    EXPECT_EQ(spy.count(), 1);
} 