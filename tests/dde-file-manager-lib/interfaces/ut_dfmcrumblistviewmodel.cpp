/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hujianzhong<hujianzhong@uniontech.com>
 *
 * Maintainer: hujianzhong<hujianzhong@uniontech.com>
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
#include <gmock/gmock-matchers.h>
#include <QFrame>

#include "interfaces/dfmcrumblistviewmodel.h"
DFM_USE_NAMESPACE
namespace  {
class TestDFMCrumbListviewModel : public testing::Test
{
public:
    DFMCrumbListviewModel *crumbListviewModel = nullptr;
    void SetUp() override
    {
        crumbListviewModel = new DFMCrumbListviewModel;

        std::cout << "start DFMCrumbListviewModel";
    }
    void TearDown() override
    {
        delete  crumbListviewModel;
        crumbListviewModel = nullptr;
        std::cout << "end DFMCrumbListviewModel";
    }
};
}
TEST_F(TestDFMCrumbListviewModel, test_removeAll)
{
    crumbListviewModel->removeAll();
}

TEST_F(TestDFMCrumbListviewModel, test_lastIndex)
{
    crumbListviewModel->lastIndex();
}
