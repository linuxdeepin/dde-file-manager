// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>

#include "stubext.h"

#include "utils/encryption/vaultconfig.h"

using namespace dfmplugin_vault;

class VaultConfigImpl : public testing::Test
{
public:
    void SetUp() override
    {
        tempDir = new QTemporaryDir;
    }

    void TearDown() override
    {
        delete tempDir;
        stub.clear();
    }

protected:
    QTemporaryDir *tempDir { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(VaultConfigImpl, ConstructWithCustomFilePath)
{
    QString filePath = tempDir->filePath("vault.ini");
    {
        VaultConfig config(filePath);
        config.set(kConfigNodeName, kConfigKeyVersion, QVariant("1050"));
    }

    VaultConfig reader(filePath);
    EXPECT_EQ(reader.get(kConfigNodeName, kConfigKeyVersion).toString(), QString("1050"));
}

TEST_F(VaultConfigImpl, SetAndGet)
{
    VaultConfig config(tempDir->filePath("vault.ini"));
    config.set("INFO", "cipher", QVariant("saltcipher"));

    EXPECT_EQ(config.get("INFO", "cipher").toString(), QString("saltcipher"));
}

TEST_F(VaultConfigImpl, GetWithDefaultValue)
{
    VaultConfig config(tempDir->filePath("vault.ini"));

    EXPECT_EQ(config.get("INFO", "missing", QVariant("default")).toString(), QString("default"));
}

TEST_F(VaultConfigImpl, VaultCreationTypeNew)
{
    VaultConfig config(tempDir->filePath("vault.ini"));
    config.setVaultCreationType(kConfigValueVaultCreationTypeNew);

    EXPECT_EQ(config.getVaultCreationType(), QString(kConfigValueVaultCreationTypeNew));
    EXPECT_TRUE(config.isNewCreated());
}

TEST_F(VaultConfigImpl, VaultCreationTypeMigrated)
{
    VaultConfig config(tempDir->filePath("vault.ini"));
    config.setVaultCreationType(kConfigValueVaultCreationTypeMigrated);

    EXPECT_EQ(config.getVaultCreationType(), QString(kConfigValueVaultCreationTypeMigrated));
    EXPECT_FALSE(config.isNewCreated());
}
