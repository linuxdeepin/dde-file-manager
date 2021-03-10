/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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

#include "interfaces/dfmviewmanager.h"
#include "views/computerview.h"

#include <gtest/gtest.h>

DFM_USE_NAMESPACE

#define viewManager DFMViewManager::instance()
namespace {
class TestDFMViewManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMSettings";
    }

    void TearDown() override
    {
        std::cout << "end TestDFMSettings";
    }
};
}

TEST_F(TestDFMViewManager, dRegisterUrlView)
{
    viewManager->dRegisterUrlView<ComputerView>("computer", "");
    EXPECT_TRUE(viewManager->isRegisted<ComputerView>("computer", ""));
}

TEST_F(TestDFMViewManager, createViewByUrl)
{
//    EXPECT_TRUE(viewManager->createViewByUrl(DUrl(COMPUTER_ROOT)) != nullptr);
}

TEST_F(TestDFMViewManager, suitedViewTypeNameByUrl)
{
    EXPECT_FALSE(viewManager->suitedViewTypeNameByUrl(DUrl(COMPUTER_ROOT)).isEmpty());
}

TEST_F(TestDFMViewManager, clearUrlView)
{
    viewManager->clearUrlView("computer", "");
    EXPECT_FALSE(viewManager->isRegisted<ComputerView>("computer", ""));
}
