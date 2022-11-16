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

#include "upgradeinterface.h"
#include "core/upgradelocker.h"
#include "core/upgradefactory.h"

#include "builtininterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace dfm_upgrade;

TEST(DoUpgrade, empty_args)
{
    EXPECT_EQ(-1, dfm_doUpgrade({}));
}

TEST(DoUpgrade, locked)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UpgradeLocker::isLock, [](){
        return true;
    });

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(1, dfm_doUpgrade(args));
}

