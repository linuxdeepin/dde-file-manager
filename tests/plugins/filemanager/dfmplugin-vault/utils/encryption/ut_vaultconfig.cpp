// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/encryption/vaultconfig.h"

#include <gtest/gtest.h>

#include <QSettings>

DPVAULT_USE_NAMESPACE

TEST(UT_VaultConfig, set)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QSettings::setValue, [ &isOk ]{
        isOk = true;
    });

    VaultConfig config;
    config.set("UT_TEST", "UT_TEST", "UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultConfig, get_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QSettings::value, []{
        return QVariant::fromValue<QString>("isOk");
    });

    VaultConfig config;
    QVariant re = config.get("UT_TEST", "UT_TEST");

    EXPECT_TRUE(re.value<QString>() == "isOk");
}

TEST(UT_VaultConfig, get_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QSettings::value, []{
        return QVariant::fromValue<QString>("isOk");
    });

    VaultConfig config;
    QVariant re = config.get("UT_TEST", "UT_TEST", "UT_TEST");

    EXPECT_TRUE(re.value<QString>() == "isOk");
}
