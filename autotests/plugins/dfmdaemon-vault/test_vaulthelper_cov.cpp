// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/daemon/vault/vaulthelper.cpp (+ vaulthelper.h inline):
//   VaultHelper::instance / VaultHelper(QObject) -> Instance_Singleton_ReturnsSameObject
//   VaultHelper::scheme (inline)                 -> Scheme_Always_ReturnsDfmvault
//   VaultHelper::isVaultFile                      -> IsVaultFile_ThreeBranches (dfmvault scheme /
//                                                    local path under mount dir / foreign url)
//   VaultHelper::vaultBaseDirLocalPath            -> BaseDirLocalPath_EndsWithVaultEncrypted
//   VaultHelper::vaultMountDirLocalPath           -> MountDirLocalPath_EndsWithVaultUnlocked
//   VaultHelper::buildVaultLocalPath              -> BuildLocalPath_EmptyBaseAndNamedBase
//   VaultHelper::vaultUrlToLocalUrl               -> VaultUrlToLocalUrl_ThreeBranches
//   VaultHelper::transUrlsToLocal                 -> TransUrlsToLocal_MixedSchemes
// Branch list:
//   isVaultFile        {scheme match, mount-path prefix, neither}
//   buildVaultLocalPath{base empty -> mount dir, base given}
//   vaultUrlToLocalUrl {non-vault scheme passthrough, path under mount dir, path appended}
//   transUrlsToLocal   {vault scheme -> convert, other scheme -> passthrough}

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "vaulthelper.h"

#include <QDir>
#include <QUrl>

DAEMONPVAULT_USE_NAMESPACE

class UT_VaultHelperCov : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override { stub.clear(); }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_VaultHelperCov, Instance_Singleton_ReturnsSameObject)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();

    // Act
    VaultHelper *again = VaultHelper::instance();

    // Assert
    EXPECT_NE(helper, nullptr);
    EXPECT_EQ(helper, again);
}

TEST_F(UT_VaultHelperCov, Scheme_OnEveryCall_ReturnsDfmvault)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();

    // Act
    QString scheme = helper->scheme();

    // Assert
    EXPECT_EQ(scheme, QString("dfmvault"));
    EXPECT_EQ(scheme.size(), static_cast<int>(8));
}

TEST_F(UT_VaultHelperCov, IsVaultFile_AllInputKinds_ClassifiedCorrectly)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();
    QUrl vaultUrl("dfmvault:///some/dir");
    QUrl localUnderMount = QUrl::fromLocalFile(helper->vaultMountDirLocalPath() + "/a.txt");
    QUrl foreignUrl = QUrl::fromLocalFile("/tmp/not-a-vault.txt");

    // Act
    bool byScheme = helper->isVaultFile(vaultUrl);
    bool byPrefix = helper->isVaultFile(localUnderMount);
    bool foreign = helper->isVaultFile(foreignUrl);

    // Assert
    EXPECT_TRUE(byScheme);
    EXPECT_TRUE(byPrefix);
    EXPECT_FALSE(foreign);
    EXPECT_EQ(foreignUrl.scheme(), QString("file"));
}

TEST_F(UT_VaultHelperCov, BaseDirLocalPath_FromConfigRoot_EndsWithVaultEncrypted)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();

    // Act
    QString baseDir = helper->vaultBaseDirLocalPath();

    // Assert
    EXPECT_TRUE(baseDir.endsWith(QStringLiteral("vault_encrypted")));
    EXPECT_EQ(baseDir, helper->buildVaultLocalPath("", QStringLiteral("vault_encrypted")));
}

TEST_F(UT_VaultHelperCov, MountDirLocalPath_FromConfigRoot_EndsWithVaultUnlocked)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();

    // Act
    QString mountDir = helper->vaultMountDirLocalPath();

    // Assert
    EXPECT_TRUE(mountDir.endsWith(QStringLiteral("vault_unlocked")));
    EXPECT_EQ(mountDir, helper->buildVaultLocalPath("", QStringLiteral("vault_unlocked")));
}

TEST_F(UT_VaultHelperCov, BuildLocalPath_EmptyOrNamedBase_PicksRightDirectory)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();

    // Act
    QString withEmptyBase = helper->buildVaultLocalPath("sub", "");
    QString withNamedBase = helper->buildVaultLocalPath("sub", "vault_encrypted");

    // Assert
    EXPECT_TRUE(withEmptyBase.endsWith(QStringLiteral("vault_unlocked/sub")));
    EXPECT_TRUE(withNamedBase.endsWith(QStringLiteral("vault_encrypted/sub")));
    EXPECT_EQ(withEmptyBase.count(QStringLiteral("vault_encrypted")), 0);   // empty base forces the mount dir
}

TEST_F(UT_VaultHelperCov, VaultUrlToLocalUrl_AllBranches_ConvertedCorrectly)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();
    const QString mountDir = helper->vaultMountDirLocalPath();
    QUrl plainFile = QUrl::fromLocalFile("/tmp/plain.txt");
    QUrl insideMount = QUrl(QString("dfmvault://%1/readme.md").arg(mountDir));
    QUrl relative = QUrl("dfmvault:///documents/a.txt");

    // Act
    QUrl passthrough = helper->vaultUrlToLocalUrl(plainFile);
    QUrl direct = helper->vaultUrlToLocalUrl(insideMount);
    QUrl appended = helper->vaultUrlToLocalUrl(relative);

    // Assert
    EXPECT_EQ(passthrough, plainFile);
    EXPECT_EQ(direct.toLocalFile(), mountDir + QStringLiteral("/readme.md"));
    EXPECT_EQ(appended.toLocalFile(), mountDir + QStringLiteral("/documents/a.txt"));
}

TEST_F(UT_VaultHelperCov, TransUrlsToLocal_MixedInput_ConvertedOrPassedThrough)
{
    // Arrange
    VaultHelper *helper = VaultHelper::instance();
    const QString mountDir = helper->vaultMountDirLocalPath();
    QList<QUrl> urls { QUrl("dfmvault:///documents/a.txt"), QUrl::fromLocalFile("/tmp/b.txt") };

    // Act
    QList<QUrl> result = helper->transUrlsToLocal(urls);

    // Assert
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).toLocalFile(), mountDir + QStringLiteral("/documents/a.txt"));
    EXPECT_EQ(result.at(1), QUrl::fromLocalFile("/tmp/b.txt"));
}
