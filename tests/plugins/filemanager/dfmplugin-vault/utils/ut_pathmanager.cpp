// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/pathmanager.h"
#include "utils/vaultdefine.h"

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_PathManager, vaultLockPath)
{
    PathManager manager;
    QString path = PathManager::vaultLockPath();

    EXPECT_TRUE(path.endsWith(kVaultEncrypyDirName));
}

TEST(UT_PathManager, vaultUnlockPath)
{
    QString path = PathManager::vaultUnlockPath();

    EXPECT_TRUE(path.endsWith(kVaultDecryptDirName));
}

TEST(UT_PathManager, makeVaultLocalPath)
{
    QString path = PathManager::makeVaultLocalPath("UT_TEST", "");

    EXPECT_TRUE(path.contains("UT_TEST"));
}

TEST(UT_PathManager, addPathSlash)
{
    QString path = PathManager::addPathSlash("UT_TEST");

    EXPECT_TRUE(path == "UT_TEST/");
}
