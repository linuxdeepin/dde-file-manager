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
#include "stubext.h"

#define private public
#include <QFile>
#include <QIODevice>
#include <views/dfmopticalmediawidget.h>
#include <views/dfmopticalmediawidget.cpp>

using namespace testing;

namespace  {
    class DFMOpticalMediaWidgetTest : public Test
    {
    public:
        virtual void SetUp() override
        {
            m_omw = new DFMOpticalMediaWidget(nullptr);
        }

        virtual void TearDown() override
        {
            delete m_omw;
        }
    public:
        DFMOpticalMediaWidget *m_omw;
    };
}



TEST_F(DFMOpticalMediaWidgetTest, updateDiscInfo)
{
    m_omw->updateDiscInfo("Disc_AA");
    auto expectValue = m_omw->d_func()->curdev == "Disc_AA";
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, setDiscMountPoint)
{
    m_omw->setDiscMountPoint("home/");
    auto expectValue = m_omw->d_func()->strMntPath == "home/";
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, setDefaultDiscName)
{
    m_omw->setDefaultDiscName("name");
    auto expectValue = "name" == m_omw->d_func()->defaultDiscName;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, hasFileInDir)
{
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    auto expectValue = true == m_omw->hasFileInDir(QDir(desktopPath));
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, getDiscMountPoint)
{
    auto expectValue = m_omw->getDiscMountPoint() == m_omw->d_func()->strMntPath;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, setBurnCapacity)
{
    //open write，打桩
    stub_ext::StubExt stub;
    bool inOpen = false;
    bool inWrite = false;
    typedef bool (*openfile)(QFile *,QFile::OpenMode);
    stub.set_lamda((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)),[&inOpen](){
        inOpen = true;
        return true;});

    typedef qint64 (*writefile)(QFile *,const QByteArray &);
    stub.set_lamda((writefile)((qint64 (QFile::*)(const QByteArray &))(&QFile::write)),[&inWrite](){
        inWrite = true;
        return true;});

    m_omw->setBurnCapacity(0,"");
    auto expectValue = m_omw->getDiscMountPoint() == m_omw->d_func()->strMntPath;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, getVolTag)
{
    DUrl fileUrl("aaa/bbb/ccc");

    EXPECT_TRUE("bbb" == m_omw->getVolTag(fileUrl));
    DUrl fileUrlEmpty("");
    EXPECT_TRUE(m_omw->getVolTag(fileUrlEmpty).isEmpty());
}

TEST_F(DFMOpticalMediaWidgetTest, hasVolProcessBusy)
{
    m_omw->g_mapCdStatusInfo.clear();
    EXPECT_FALSE(m_omw->hasVolProcessBusy());
    CdStatusInfo data1;
    CdStatusInfo data2;
    data1.bProcessLocked = false;
    data2.bProcessLocked = false;
    m_omw->g_mapCdStatusInfo["one"] = data1;
    m_omw->g_mapCdStatusInfo["two"] = data2;

    EXPECT_FALSE(m_omw->hasVolProcessBusy());
    m_omw->g_mapCdStatusInfo["one"].bProcessLocked = true;
    m_omw->g_mapCdStatusInfo["two"].bProcessLocked = true;
    EXPECT_TRUE(m_omw->hasVolProcessBusy());
}

TEST_F(DFMOpticalMediaWidgetTest, getCdStatusInfo)
{
    m_omw->g_mapCdStatusInfo.clear();
    auto expectV = nullptr == m_omw->getCdStatusInfo("aaa");
    EXPECT_TRUE(expectV);
    CdStatusInfo data1;
    CdStatusInfo data2;
    data1.bProcessLocked = false;
    data2.bProcessLocked = false;
    m_omw->g_mapCdStatusInfo["one"] = data1;
    m_omw->g_mapCdStatusInfo["two"] = data2;
    auto expectValue = nullptr != m_omw->getCdStatusInfo("one");
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, test_pb_burn_button_clicked)
{
    stub_ext::StubExt stub;
    bool inThere = false;
    stub.set_lamda(ADDR(DFMOpticalMediaWidgetPrivate, getVolTag), [&inThere](){
        inThere = true;
        return "";
    });

    emit m_omw->d_func()->pb_burn->clicked();
    EXPECT_TRUE(inThere);

}


