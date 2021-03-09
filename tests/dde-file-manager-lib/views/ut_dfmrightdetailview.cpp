/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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

#define private public
#include <views/dfmrightdetailview.h>
#include <views/dfmrightdetailview.cpp>
#include <QStandardPaths>

TEST(DFMRightDetailViewTest, setUrl){
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    DUrl temp = DUrl();
    DFMRightDetailView ddv(temp);

    EXPECT_TRUE(ddv.d_func()->m_url.isEmpty());

    ddv.setUrl(desktopUrl);
    auto expectValue = ddv.d_func()->m_url == desktopUrl;
    EXPECT_TRUE(expectValue);
}

TEST(DFMRightDetailViewTest, setTagWidgetVisible){
    DUrl temp = DUrl();
    DFMRightDetailView ddv(temp);

    auto tagVisible = ddv.d_func()->tagInfoWidget->isVisible();
    if(tagVisible){
        ddv.setTagWidgetVisible(false);
        EXPECT_TRUE(tagVisible != false);
    }
    else {
        ddv.setTagWidgetVisible(true);
        EXPECT_TRUE(tagVisible != true);
    }
}
