/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
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
#include "controllers/dfmrecentcrumbcontroller.h"

using DFM_NAMESPACE::DFMRecentCrumbController;
using DFM_NAMESPACE::CrumbData;

namespace  {
    class DFMRecentCrumbControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMRecentCrumbController controller;
    };
}

TEST_F(DFMRecentCrumbControllerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(RECENT_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}

TEST_F(DFMRecentCrumbControllerTest, seprateUrl){
     DUrl url;
     url.setScheme(RECENT_SCHEME);
     QList<CrumbData> list = controller.seprateUrl(url);
     EXPECT_TRUE(!list.empty());
}
