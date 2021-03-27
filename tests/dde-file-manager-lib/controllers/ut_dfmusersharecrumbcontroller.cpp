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

#include <gtest/gtest.h>
#include <QDateTime>
#include <dfmevent.h>

#include "controllers/dfmusersharecrumbcontroller.h"

using namespace testing;
DFM_USE_NAMESPACE

class DFMUserShareCrumbControllerTest:public testing::Test{

public:
    DFMUserShareCrumbController controller;
    virtual void SetUp() override{
        std::cout << "start DFMUserShareCrumbControllerTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFMUserShareCrumbControllerTest" << std::endl;
    }
};


TEST_F(DFMUserShareCrumbControllerTest,can_supportedUrl){
    DUrl url;
    url.setScheme(USERSHARE_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}

TEST_F(DFMUserShareCrumbControllerTest,can_seprateUrl){
    DUrl url;
    url.setScheme(USERSHARE_SCHEME);
    EXPECT_TRUE(!controller.seprateUrl(url).isEmpty());
}
