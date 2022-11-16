/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    EXPECT_TRUE(isNeedUpgrade());

    retDir = "/home";
    EXPECT_FALSE(isNeedUpgrade());
}
