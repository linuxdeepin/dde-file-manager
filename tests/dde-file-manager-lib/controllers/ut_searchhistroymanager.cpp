/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include <gtest/gtest.h>

#include "controllers/searchhistroymanager.h"

namespace  {
class TestSearchHistroyManager : public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestSearchHistroyManager" << std::endl;
        manager = new SearchHistroyManager();
    }

    virtual void TearDown() override
    {
        std::cout << "end TestSearchHistroyManager" << std::endl;
        if (manager) {
            delete manager;
            manager = nullptr;
        }
    }

public:
    SearchHistroyManager *manager;
};
}

TEST_F(TestSearchHistroyManager, toStringList)
{
    manager->writeIntoSearchHistory("keyworld");
    manager->writeIntoSearchHistory("");
    auto res = manager->toStringList();
    EXPECT_TRUE(!res.isEmpty());
}

TEST_F(TestSearchHistroyManager, removeSearchHistory)
{
    auto res = manager->removeSearchHistory("");
    EXPECT_FALSE(res);

    res = manager->removeSearchHistory("not exist");
    EXPECT_FALSE(res);

    res = manager->removeSearchHistory("keyworld");
    EXPECT_TRUE(res);
}

TEST_F(TestSearchHistroyManager, clearHistory)
{
    manager->clearHistory();
    auto res = manager->toStringList();
    EXPECT_TRUE(res.isEmpty());
}
