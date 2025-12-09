// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "builtininterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace dfm_upgrade;

TEST(BuiltIn, upgradeConfigDir)
{
    auto path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/.config/deepin/dde-file-manager";
    EXPECT_EQ(path, upgradeConfigDir());
}

TEST(BuiltIn, isNeedUpgrade)
{
    const QString targetDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/.config/deepin/dde-file-manager";
    const QString path = targetDir + "/dfm-upgraded.lock";
    QString retDir = targetDir;
    stub_ext::StubExt stub;
    stub.set_lamda(&upgradeConfigDir, [&retDir](){
        return retDir;
    });

    stub.set_lamda((bool (*)(const QString &))&QFile::exists, [path](const QString &file) {
        return file == path;
    });

    bool wri = false;
    stub.set_lamda(&QFileInfo::isWritable, [&wri]() {
        return wri;
    });

    EXPECT_FALSE(isNeedUpgrade());

    wri = true;
    EXPECT_TRUE(isNeedUpgrade());

    retDir = "/home";
    EXPECT_FALSE(isNeedUpgrade());
}
