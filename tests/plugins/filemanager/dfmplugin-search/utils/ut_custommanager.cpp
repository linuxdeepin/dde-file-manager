/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "utils/custommanager.h"
#include "utils/searchhelper.h"

#include "stubext.h"

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE

TEST(CustomManagerTest, ut_registerCustomInfo)
{
    QVariantMap data;
    EXPECT_TRUE(CustomManager::instance()->registerCustomInfo("test", data));
    EXPECT_FALSE(CustomManager::instance()->registerCustomInfo("test", data));
}

TEST(CustomManagerTest, ut_isRegisted)
{
    EXPECT_TRUE(CustomManager::instance()->isRegisted("test"));
}

TEST(CustomManagerTest, ut_isDisableSearch)
{
    CustomManager::instance()->customInfos.clear();

    QVariantMap data;
    data.insert("Property_Key_DisableSearch", true);
    CustomManager::instance()->registerCustomInfo("test", data);

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromUserInput("test:///home"), "test", "123");
    EXPECT_TRUE(CustomManager::instance()->isDisableSearch(url));
}

TEST(CustomManagerTest, ut_redirectedPath)
{
    CustomManager::instance()->customInfos.clear();

    QVariantMap data;
    data.insert("Property_Key_RedirectedPath", "/home");
    CustomManager::instance()->registerCustomInfo("test", data);

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromUserInput("test:///home"), "test", "123");
    auto path = CustomManager::instance()->redirectedPath(url);
    EXPECT_TRUE(!path.isEmpty());
}
