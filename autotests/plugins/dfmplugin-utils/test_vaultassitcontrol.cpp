// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/vaultassist/vaultassitcontrol.h"

#include <dfm-io/dfmio_utils.h>

#include <QDir>
#include <QUrl>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_VaultAssitControl : public testing::Test
{
protected:
    void SetUp() override
    {
        control = VaultAssitControl::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    VaultAssitControl *control { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_VaultAssitControl, instance_ReturnsSingleton)
{
    VaultAssitControl *instance1 = VaultAssitControl::instance();
    VaultAssitControl *instance2 = VaultAssitControl::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_VaultAssitControl, scheme_ReturnsDfmvault)
{
    QString result = control->scheme();

    EXPECT_EQ(result, "dfmvault");
}

TEST_F(UT_VaultAssitControl, isVaultFile_VaultScheme_ReturnsTrue)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/test/file.txt");

    bool result = control->isVaultFile(vaultUrl);

    EXPECT_TRUE(result);
}

TEST_F(UT_VaultAssitControl, isVaultFile_NonVaultScheme_ReturnsFalse)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/test.txt");

    bool result = control->isVaultFile(fileUrl);

    EXPECT_FALSE(result);
}

TEST_F(UT_VaultAssitControl, isVaultFile_PathInMountDir_ReturnsTrue)
{
    QUrl fileUrl = QUrl::fromLocalFile("/home/test/.config/Vault/vault_unlocked/test.txt");

    bool result = control->isVaultFile(fileUrl);

    EXPECT_TRUE(result);
}

TEST_F(UT_VaultAssitControl, vaultBaseDirLocalPath_ReturnsPath)
{
    QString result = control->vaultBaseDirLocalPath();

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(UT_VaultAssitControl, vaultMountDirLocalPath_ReturnsPath)
{
    QString result = control->vaultMountDirLocalPath();

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(UT_VaultAssitControl, buildVaultLocalPath_EmptyBase_UsesDefault)
{
    QString result = control->buildVaultLocalPath("test", "");

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(UT_VaultAssitControl, buildVaultLocalPath_WithBase_UsesBase)
{
    QString result = control->buildVaultLocalPath("test", "custom_base");

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(UT_VaultAssitControl, vaultUrlToLocalUrl_NonVaultScheme_ReturnsOriginal)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/test.txt");

    QUrl result = control->vaultUrlToLocalUrl(fileUrl);

    EXPECT_EQ(result, fileUrl);
}

TEST_F(UT_VaultAssitControl, vaultUrlToLocalUrl_VaultScheme_ReturnsLocalUrl)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/test/file.txt");

    QUrl result = control->vaultUrlToLocalUrl(vaultUrl);

    EXPECT_EQ(result.scheme(), "file");
}

TEST_F(UT_VaultAssitControl, transUrlsToLocal_MixedUrls_TransformsVaultOnly)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/vault/file.txt");

    QUrl localUrl = QUrl::fromLocalFile("/tmp/local.txt");

    QList<QUrl> urls = { vaultUrl, localUrl };

    QList<QUrl> result = control->transUrlsToLocal(urls);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].scheme(), "file");
    EXPECT_EQ(result[1], localUrl);
}

TEST_F(UT_VaultAssitControl, transUrlsToLocal_EmptyList_ReturnsEmpty)
{
    QList<QUrl> urls;

    QList<QUrl> result = control->transUrlsToLocal(urls);

    EXPECT_TRUE(result.isEmpty());
}
