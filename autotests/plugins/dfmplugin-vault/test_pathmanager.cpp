// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>

#include "stubext.h"

#include "utils/pathmanager.h"
#include "utils/vaultdefine.h"

#include <dfm-base/utils/dialogmanager.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class PathManagerTest : public testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(PathManagerTest, Construct_AsQObject_NoParent)
{
    PathManager *mgr = new PathManager();
    ASSERT_NE(mgr, nullptr);
    EXPECT_EQ(mgr->parent(), nullptr);
    delete mgr;
}

TEST_F(PathManagerTest, VaultPswContainerPath_AppendsSeparatorAndName)
{
    QString path = PathManager::vaultPswContainerPath("/tmp/vault_base");
    EXPECT_EQ(path, QString("/tmp/vault_base") + QDir::separator() + QString(kVaultPswContainerFileName));
}

TEST_F(PathManagerTest, VaultEncryptPath_AppendsEncryptDirName)
{
    QString path = PathManager::vaultEncryptPath("/tmp/vault_base");
    EXPECT_EQ(path, QString("/tmp/vault_base") + QDir::separator() + QString(kVaultEncrypyDirName));
}

TEST_F(PathManagerTest, VaultMountPath_AppendsDecryptDirName)
{
    QString path = PathManager::vaultMountPath("/tmp/vault_base");
    EXPECT_EQ(path, QString("/tmp/vault_base") + QDir::separator() + QString(kVaultDecryptDirName));
}

TEST_F(PathManagerTest, AddPathSlash_AppendsTrailingSlash)
{
    QString path = PathManager::addPathSlash("/tmp/vault_dir");
    EXPECT_EQ(path, QString("/tmp/vault_dir/"));
}

TEST_F(PathManagerTest, CreateDirIfNotExist_DirAbsent_CreatesDir)
{
    tempDir = std::make_unique<QTemporaryDir>();
    ASSERT_TRUE(tempDir->isValid());
    QString target = tempDir->path() + "/fresh_dir";

    EXPECT_FALSE(QFile::exists(target));
    ASSERT_TRUE(PathManager::createDirIfNotExist(target));
    EXPECT_TRUE(QDir(target).exists());
}

TEST_F(PathManagerTest, CreateDirIfNotExist_EmptyDir_ReturnsTrue)
{
    tempDir = std::make_unique<QTemporaryDir>();
    ASSERT_TRUE(tempDir->isValid());

    EXPECT_TRUE(PathManager::createDirIfNotExist(tempDir->path()));
}

TEST_F(PathManagerTest, CreateDirIfNotExist_NonEmptyDir_ReturnsFalse)
{
    tempDir = std::make_unique<QTemporaryDir>();
    ASSERT_TRUE(tempDir->isValid());
    QFile f(tempDir->path() + "/occupied");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    EXPECT_FALSE(PathManager::createDirIfNotExist(tempDir->path()));
}

TEST_F(PathManagerTest, CreateVaultMountDir_FreshBase_CreatesMountDir)
{
    tempDir = std::make_unique<QTemporaryDir>();
    ASSERT_TRUE(tempDir->isValid());

    ASSERT_TRUE(PathManager::createVaultMountDir(tempDir->path()));
    EXPECT_TRUE(QDir(PathManager::vaultMountPath(tempDir->path())).exists());
}

TEST_F(PathManagerTest, CreateVaultMountDir_OccupiedMountDir_ShowsError)
{
    tempDir = std::make_unique<QTemporaryDir>();
    ASSERT_TRUE(tempDir->isValid());

    QString mountDir = PathManager::vaultMountPath(tempDir->path());
    ASSERT_TRUE(QDir().mkpath(mountDir));
    QFile f(mountDir + "/occupied");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    bool dialogShown = false;
    stub.set_lamda(VADDR(DialogManager, showErrorDialog),
                   [&dialogShown](DialogManager *, const QString &, const QString &) {
                       dialogShown = true;
                   });

    EXPECT_FALSE(PathManager::createVaultMountDir(tempDir->path()));
    EXPECT_TRUE(dialogShown);
}
