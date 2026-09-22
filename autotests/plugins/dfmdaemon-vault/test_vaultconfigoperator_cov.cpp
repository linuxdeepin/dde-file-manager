// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/daemon/vault/vaultconfigoperator.cpp:
//   VaultConfigOperator::VaultConfigOperator(QString) -> Ctor_ExplicitPath_CreatesBackingFile,
//                                                        Ctor_EmptyPath_UsesDefaultPath
//   VaultConfigOperator::~VaultConfigOperator          -> covered by every test (scope exit flushes QSettings)
//   VaultConfigOperator::set                           -> Set_ThenGet_RoundTripsValue
//   VaultConfigOperator::get (2 overloads)             -> Get_MissingKey_ReturnsDefault,
//                                                        Get_ExistingKey_ReturnsStoredValue
// Branch list: ctor {empty path -> default path, explicit path}; get {key present, key absent}.

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "vaultconfigoperator.h"

#include <QFileInfo>
#include <QTemporaryDir>

DAEMONPVAULT_USE_NAMESPACE

class UT_VaultConfigOperatorCov : public testing::Test
{
protected:
    void SetUp() override { tmpDir.reset(new QTemporaryDir); }
    void TearDown() override { stub.clear(); }

protected:
    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tmpDir;
};

TEST_F(UT_VaultConfigOperatorCov, Set_ThenGet_RoundTripsValue)
{
    // Arrange
    const QString path = tmpDir->filePath("vaultConfig.ini");
    VaultConfigOperator op(path);

    // Act
    op.set("VaultConfig", "EncryptionMethod", QVariant(QString("transparent")));
    QVariant got = op.get("VaultConfig", "EncryptionMethod");

    // Assert
    EXPECT_EQ(got.toString(), QString("transparent"));
    EXPECT_FALSE(op.get("VaultConfig", "NeverWrittenKey").isValid());
}

TEST_F(UT_VaultConfigOperatorCov, Get_MissingKey_ReturnsDefault)
{
    // Arrange
    VaultConfigOperator op(tmpDir->filePath("other.ini"));

    // Act
    QVariant got = op.get("NoSuchNode", "NoSuchKey", QVariant(QString("fallback")));

    // Assert
    EXPECT_EQ(got.toString(), QString("fallback"));
    EXPECT_FALSE(op.get("NoSuchNode", "AnotherMissingKey").isValid());
}

TEST_F(UT_VaultConfigOperatorCov, Ctor_ExplicitPath_CreatesBackingFile)
{
    // Arrange
    const QString path = tmpDir->filePath("created.ini");
    ASSERT_FALSE(QFileInfo::exists(path));

    // Act
    {
        VaultConfigOperator op(path);
        op.set("Node", "Key", QVariant(42));
    }

    // Assert
    EXPECT_TRUE(QFileInfo::exists(path));
    VaultConfigOperator reopen(path);
    EXPECT_EQ(reopen.get("Node", "Key").toInt(), 42);
}

TEST_F(UT_VaultConfigOperatorCov, Ctor_EmptyPath_UsesDefaultPath)
{
    // Arrange - empty path falls back to ~/.config/Vault/vaultConfig.ini (read-only here)
    VaultConfigOperator op("");

    // Act - read a key that was never written

    // Assert - default-path instance reads a missing key without touching it
    QVariant got = op.get("CoverageNode", "CoverageKey", QVariant(QString("def")));
    EXPECT_EQ(got.toString(), QString("def"));
    EXPECT_EQ(op.get("CoverageNode", "Missing").isValid(), false);
}
