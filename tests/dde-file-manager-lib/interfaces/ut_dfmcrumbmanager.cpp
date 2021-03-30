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

#include "interfaces/dfmcrumbmanager.h"
#include "controllers/dfmmdcrumbcontrooler.h"
#include "controllers/dfmfilecrumbcontroller.h"
#include <gtest/gtest.h>
#include <QFrame>
#include "dfmcrumbbar.h"
DFM_USE_NAMESPACE

namespace {
class TestDFMCrumbManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMCrumbManager";
    }

    void TearDown() override
    {
        std::cout << "end TestDFMCrumbManager";
    }
};
}

TEST_F(TestDFMCrumbManager, isRegisted)
{
    DFMCrumbManager::instance()->dRegisterCrumbCreator<DFMFileCrumbController>(QStringLiteral(FILE_SCHEME));
    EXPECT_TRUE(DFMCrumbManager::instance()->isRegisted<DFMFileCrumbController>(QStringLiteral(FILE_SCHEME)));
}

TEST_F(TestDFMCrumbManager, createControllerByUrl)
{
    DUrl newurl(DUrl::fromLocalFile("/home"));
    DFMCrumbInterface *crumbController = nullptr;
    QWidget *qframe = new QWidget();
    DFMCrumbBar *p_tr = new DFMCrumbBar(qframe);
    crumbController = DFMCrumbManager::instance()->createControllerByUrl(newurl, p_tr);
    EXPECT_TRUE(crumbController != nullptr);

    delete qframe;
    qframe = nullptr;

    delete crumbController;
    crumbController = nullptr;
}

