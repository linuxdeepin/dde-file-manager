// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "units/dconfigupgradeunit.h"
#include "utils/upgradeutils.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QVariant>
#include <QStringList>
#include <QJsonArray>

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

namespace {
// captured calls of DConfigManager stubs
struct DConfigSpy {
    int setValueCount = 0;
    QList<QPair<QString, QString>> setValueKeys;
    QVariant lastSetValue;
    int addConfigResult = 1;   // how many times addConfig succeeded (0 -> fail)
    int addConfigCalls = 0;
    QVariant valueToReturn;
    int valueCalls = 0;
    // per-key return values for UpgradeUtils::genericAttribute
    QHash<QString, QVariant> genericValues;
};
DConfigSpy *g_spy = nullptr;
}   // namespace

class DConfigUpgradeUnitTest : public testing::Test
{
protected:
    void SetUp() override
    {
        spy = new DConfigSpy;
        g_spy = spy;
        unit = new DConfigUpgradeUnit();
    }

    void TearDown() override
    {
        delete unit;
        unit = nullptr;
        g_spy = nullptr;
        delete spy;
        stub.clear();
    }

    // stub DConfigManager::value / setValue / addConfig and record invocations
    void installDConfigStubs()
    {
        using ValueSig = QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const;
        stub.set_lamda(static_cast<ValueSig>(&DConfigManager::value),
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           if (g_spy)
                               g_spy->valueCalls++;
                           return g_spy ? g_spy->valueToReturn : QVariant();
                       });
        stub.set_lamda(ADDR(DConfigManager, setValue),
                       [](DConfigManager *, const QString &cfg, const QString &key, const QVariant &val) {
                           __DBG_STUB_INVOKE__
                           if (g_spy) {
                               g_spy->setValueCount++;
                               g_spy->setValueKeys.append({ cfg, key });
                               g_spy->lastSetValue = val;
                           }
                       });
        stub.set_lamda(ADDR(DConfigManager, addConfig),
                       [](DConfigManager *, const QString &, QString *) -> bool {
                           __DBG_STUB_INVOKE__
                           if (g_spy)
                               g_spy->addConfigCalls++;
                           return g_spy ? g_spy->addConfigResult != 0 : true;
                       });
    }

    // control what UpgradeUtils::genericAttribute returns (per key, with a fallback)
    void installGenericStub(const QVariant &fallback, const QHash<QString, QVariant> &perKey = {})
    {
        spy->genericValues = perKey;
        const QVariant boundFallback = fallback;
        stub.set_lamda(ADDR(UpgradeUtils, genericAttribute),
                       [this, boundFallback](const QString &key) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return spy->genericValues.value(key, boundFallback);
                       });
    }

    DConfigUpgradeUnit *unit { nullptr };
    DConfigSpy *spy { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(DConfigUpgradeUnitTest, MappedActions_ContainsKnownActionMappings)
{
    // Arrange / Act
    const QMap<QString, QString> &mapped = unit->mappedActions();

    // Assert
    EXPECT_EQ(mapped.value("Open"), QString("open"));
    EXPECT_EQ(mapped.value("OpenDisk"), QString("computer-open"));
    EXPECT_EQ(mapped.value("ClearTrash"), QString("empty-trash"));
    EXPECT_TRUE(mapped.contains("Compress"));
}

TEST_F(DConfigUpgradeUnitTest, UpgradeMenuConfigs_MapsActionsAndWritesBack)
{
    // Arrange
    installDConfigStubs();
    installGenericStub(QVariant());
    spy->valueToReturn = QVariant(QStringList { QString("Open"), QString("NotMappedAction") });

    // Act
    const bool ok = unit->upgradeMenuConfigs();

    // Assert
    EXPECT_TRUE(ok);
    // three hidden-lists are written back to dconfig
    EXPECT_GE(spy->setValueCount, 3);
    bool foundMapped = false;
    bool foundUnmapped = false;
    for (const QVariant &v : { spy->lastSetValue }) {
        const QStringList list = v.toStringList();
        foundMapped |= list.contains(QString("open"));
        foundUnmapped |= list.contains(QString("NotMappedAction"));
    }
    EXPECT_TRUE(foundMapped);
    EXPECT_TRUE(foundUnmapped);
}

TEST_F(DConfigUpgradeUnitTest, UpgradeSmbConfigs_OldValueValid_WritesAndRegisters)
{
    // Arrange
    installDConfigStubs();
    installGenericStub(QVariant(true));

    // Act
    const bool ok = unit->upgradeSmbConfigs();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(unit->oldGenericSettings.contains("AlwaysShowOfflineRemoteConnections"));
    EXPECT_GE(spy->setValueCount, 1);
}

TEST_F(DConfigUpgradeUnitTest, UpgradeSmbConfigs_AlreadyProcessedOrInvalid_Skips)
{
    // Arrange: OldAttributes says the key was processed -> early skip
    installDConfigStubs();
    installGenericStub(QVariant(),
                       { { "OldAttributes",
                           QVariant(QStringList { QString("AlwaysShowOfflineRemoteConnections") }) } });

    // Act
    const bool ok = unit->upgradeSmbConfigs();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(unit->oldGenericSettings.isEmpty());
}

TEST_F(DConfigUpgradeUnitTest, UpgradeRecentConfigs_AddConfigFails_ReturnsFalse)
{
    // Arrange
    installDConfigStubs();
    installGenericStub(QVariant(false));
    spy->addConfigResult = 0;

    // Act
    const bool ok = unit->upgradeRecentConfigs();

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_FALSE(unit->oldGenericSettings.contains("ShowRecentFileEntry"));
}

TEST_F(DConfigUpgradeUnitTest, UpgradeRecentConfigs_Successful_MigratesSidebarVisibility)
{
    // Arrange
    installDConfigStubs();
    installGenericStub(QVariant(true));
    QVariantMap sideBar;
    sideBar.insert("recent", false);
    spy->valueToReturn = QVariant(sideBar);

    // Act
    const bool ok = unit->upgradeRecentConfigs();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(unit->oldGenericSettings.contains("ShowRecentFileEntry"));
    EXPECT_GE(spy->setValueCount, 1);
}

TEST_F(DConfigUpgradeUnitTest, UpgradeSearchConfigs_SuccessfulAndFailureBranches)
{
    // Arrange: success path
    installDConfigStubs();
    installGenericStub(QVariant(true));

    // Act
    const bool ok = unit->upgradeSearchConfigs();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(unit->oldGenericSettings.contains("IndexFullTextSearch"));

    // Arrange: dconfig file cannot be added
    spy->addConfigResult = 0;
    DConfigUpgradeUnit failing;

    // Act
    const bool failed = failing.upgradeSearchConfigs();

    // Assert
    EXPECT_FALSE(failed);
}

TEST_F(DConfigUpgradeUnitTest, ClearDiskHidden_WritesEmptyList)
{
    // Arrange
    installDConfigStubs();

    // Act
    unit->clearDiskHidden();

    // Assert
    EXPECT_EQ(spy->setValueCount, 1);
    EXPECT_TRUE(spy->lastSetValue.toStringList().isEmpty());
}

TEST_F(DConfigUpgradeUnitTest, AddOldGenericSettings_OnlyCalledWhenNonEmpty)
{
    // Arrange
    int addOldCalls = 0;
    stub.set_lamda(ADDR(UpgradeUtils, addOldGenericAttribute),
                   [&addOldCalls](const QJsonArray &) {
                       __DBG_STUB_INVOKE__
                       addOldCalls++;
                   });

    // Act
    unit->addOldGenericSettings();
    const int callsWhenEmpty = addOldCalls;
    unit->oldGenericSettings.append("SomeKey");
    unit->addOldGenericSettings();

    // Assert
    EXPECT_EQ(callsWhenEmpty, 0);
    EXPECT_EQ(addOldCalls, 1);
}

TEST_F(DConfigUpgradeUnitTest, CheckOldGeneric_DependsOnOldAttributesValue)
{
    // Arrange
    installGenericStub(QVariant(),
                       { { "OldAttributes", QVariant(QStringList { QString("KeyA") }) } });

    // Act
    const bool processed = unit->checkOldGeneric("KeyA");
    const bool notProcessed = unit->checkOldGeneric("KeyB");

    // Assert
    EXPECT_TRUE(processed);
    EXPECT_FALSE(notProcessed);
}

TEST_F(DConfigUpgradeUnitTest, Upgrade_AllStepsExecuted_ReturnsTrue)
{
    // Arrange
    installDConfigStubs();
    installGenericStub(QVariant(true));

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_GT(spy->valueCalls, 0);
    EXPECT_GT(spy->setValueCount, 0);
}
