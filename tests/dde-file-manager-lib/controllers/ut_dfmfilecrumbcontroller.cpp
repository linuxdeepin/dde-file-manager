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

#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#define private public
#include <QDesktopServices>
#include "controllers/dfmfilecrumbcontroller.h"
DFM_USE_NAMESPACE



namespace  {

class TestDFMFileCrumbController : public testing::Test
{
public:
    DFMFileCrumbController dfmFileCrubCtl;
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

}

TEST_F(TestDFMFileCrumbController, tst_supportedUrl)
{
    DUrl url("file:///home");
    EXPECT_TRUE(dfmFileCrubCtl.supportedUrl(url));
}

TEST_F(TestDFMFileCrumbController, tst_seprateUrl)
{
    DUrl url("file:///home");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}

TEST_F(TestDFMFileCrumbController, tst_seprateUrl2)
{
    DUrl url("file:///tmp");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
TEST_F(TestDFMFileCrumbController, tst_seprateUrl3)
{
    DUrl url("file:///");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
TEST_F(TestDFMFileCrumbController, tst_seprateUrl4)
{
    QString filename;
    DUrl url("file:///media");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}

TEST_F(TestDFMFileCrumbController, tst_getDisplayName)
{
    QProcess::execute("toutch /tmp/1234.txt");
    QString filename("/tmp/1234.txt");
    QString getFilename = dfmFileCrubCtl.getDisplayName(filename);
    QProcess::execute("rm /tmp/1234.txt -f");
}
