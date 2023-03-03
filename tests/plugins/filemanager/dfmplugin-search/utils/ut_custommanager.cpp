// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
