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
#include "stub.h"
#include "../stub-ext/stubext.h"


#define private public
#define protected public
#include <mediainfo/dfmmediainfo.h>
#include <views/dfmfilebasicinfowidget.h>
#include <views/dfmfilebasicinfowidget.cpp>
#include <QStandardPaths>
#include <QEvent>
#include <QFile>

using namespace testing;
DFM_BEGIN_NAMESPACE
namespace  {
    class DFMFileBasicInfoWidgetTest : public Test
    {
    public:
        virtual void SetUp() override
        {
            m_biw = new DFMFileBasicInfoWidget(nullptr);
        }

        virtual void TearDown() override
        {
            delete  m_biw;
            m_biw = nullptr;
        }
    public:
        DFMFileBasicInfoWidget *m_biw;
    };

}

TEST_F(DFMFileBasicInfoWidgetTest, setUrl)
{

    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    EXPECT_TRUE(m_biw->d_func()->m_url.isEmpty());
    m_biw->setShowFileName(true);
    m_biw->setShowMediaInfo(true);
    m_biw->setUrl(desktopUrl);
    auto expectValue = m_biw->d_func()->m_url == desktopUrl;
    EXPECT_TRUE(expectValue);

    QString imageUrl = QString("%1/forUtTest.jpg").arg(desktopPath);
    QString imageLink = QString("%1/forUtTest_link.jpg").arg(desktopPath);

    QFile imageFile(imageUrl);
    if (!imageFile.exists()) {
        ASSERT_TRUE(imageFile.open(QIODevice::WriteOnly));//创建一个测试图片备用
        ASSERT_TRUE(imageFile.link(imageLink));//创建一个测试图片link文件备用
    }
    stub_ext::StubExt stu;
    QPointer<DFMMediaInfo> tg = nullptr;
    stu.set_lamda(ADDR(DFMMediaInfo, startReadInfo), [&tg](void* obj){tg = static_cast<DFMMediaInfo*>(obj);});
    //普通图片
    m_biw->setShowFileName(true);
    m_biw->setShowMediaInfo(true);
    m_biw->d_func()->m_url = DUrl("");
    m_biw->setUrl(DUrl(QString("file://%1").arg(imageUrl)));
    if (!tg.isNull()) {
        emit tg->Finished();
    }

    auto expectValue2 = m_biw->d_func()->m_url == DUrl(QString("file://%1").arg(imageUrl));
    EXPECT_TRUE(expectValue2);

    //链接文件
    m_biw->setShowFileName(true);
    m_biw->setShowMediaInfo(true);
    m_biw->d_func()->m_url = DUrl("");
    m_biw->setUrl(DUrl(QString("file://%1").arg(imageLink)));

    auto expectValueLink = m_biw->d_func()->m_url == DUrl(QString("file://%1").arg(imageLink));
    EXPECT_TRUE(expectValueLink);

    //移除为ut测试创建的文件
    imageFile.remove(imageUrl);
    imageFile.remove(imageLink);
    imageFile.close();

    //回收站文件
    desktopPath = desktopPath.left(desktopPath.size() - 8);
    QString userName = desktopPath.split("/").last();
    QString targetPath = QString("/home/%1/.local/share/Trash/files/forUtTest.jpg").arg(userName);
    QFile TrushFile(targetPath);
    if (!imageFile.exists()) {
        ASSERT_TRUE(TrushFile.open(QIODevice::WriteOnly));//创建一个测试图片备用
    }

    m_biw->setShowFileName(true);
    m_biw->setShowMediaInfo(true);
    m_biw->d_func()->m_url = DUrl("");
    m_biw->setUrl(DUrl(QString("trash://%1").arg(targetPath)));

    auto expectValueTrush = m_biw->d_func()->m_url == DUrl(QString("trash://%1").arg(targetPath));
    EXPECT_TRUE(expectValueTrush);
    TrushFile.remove(targetPath);
    TrushFile.close();
}

TEST_F(DFMFileBasicInfoWidgetTest, showFileName)
{
    auto expectValue = m_biw->showFileName() == m_biw->d_func()->m_showFileName;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, setShowFileName)
{
    m_biw->setShowFileName(false);
    auto expectValue = false == m_biw->d_func()->m_showFileName;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, showMediaInfo)
{
    auto expectValue = m_biw->showMediaInfo() == m_biw->d_func()->m_showMediaInfo;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, setShowMediaInfo)
{
    m_biw->setShowMediaInfo(false);
    auto expectValue = false == m_biw->d_func()->m_showMediaInfo;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, showSummary)
{
    auto expectValue = m_biw->showSummary() == m_biw->d_func()->m_showSummaryOnly;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, setShowSummary)
{
    m_biw->setShowSummary(false);
    auto expectValue = false == m_biw->d_func()->m_showSummaryOnly;
    EXPECT_TRUE(expectValue);
}

//TEST_F(DFMFileBasicInfoWidgetTest, updateSizeText){
//    auto expectValue = m_biw->showFileName() == m_biw->d_func()->m_showFileName;
//    EXPECT_TRUE(expectValue);
//}


TEST_F(DFMFileBasicInfoWidgetTest, startCalcFolderSize)
{
    Stub stub;
    auto ut_start = static_cast<void(*)()>([](){});
    auto target = static_cast<void(DFileStatisticsJob::*)(const DUrlList &)>(ADDR(DFileStatisticsJob, start));
    stub.set(target, ut_start);

    m_biw->d_func()->m_showSummaryOnly = true;
    m_biw->d_func()->m_sizeWorker = nullptr;
    m_biw->d_func()->startCalcFolderSize();
    EXPECT_EQ(nullptr, m_biw->d_func()->m_sizeWorker);

    m_biw->d_func()->m_url = DUrl("file:///home");
    m_biw->d_func()->m_showSummaryOnly = false;
    m_biw->d_func()->startCalcFolderSize();
    EXPECT_NE(nullptr, m_biw->d_func()->m_sizeWorker);
}

TEST(SectionValueLabel, test_event){
    QEvent et(QEvent::FontChange);
    SectionValueLabel svlab;
    svlab.setText("aa");
    auto lthaa = svlab.width();
    svlab.setText("aaaa");
    svlab.event(&et);
    auto lthaaaa = svlab.width();
    EXPECT_FALSE(lthaa == lthaaaa);
}

TEST(SectionValueLabel, section_value_label){
    QShowEvent se;
    SectionValueLabel svlab;
    svlab.setText("aa");
    auto lthaa = svlab.width();
    svlab.setText("aaaa");
    svlab.showEvent(&se);
    auto lthaaaa = svlab.width();
    EXPECT_FALSE(lthaa == lthaaaa);
}

TEST(LinkSectionValueLabel, link_target_url){
    LinkSectionValueLabel lsvlab;
    lsvlab.m_linkTargetUrl = DUrl("/home/");
    auto expectValue =DUrl("/home/") == lsvlab.linkTargetUrl();
    EXPECT_TRUE(expectValue);
}

TEST(LinkSectionValueLabel, set_link_target_url){
    LinkSectionValueLabel lsvlab;
    lsvlab.setLinkTargetUrl(DUrl("/home/"));
    auto expectValue =DUrl("/home/") == lsvlab.m_linkTargetUrl;
    EXPECT_TRUE(expectValue);
}

TEST(LinkSectionValueLabel, mouseReleaseEvent)
{
    bool inThere = false;
    stub_ext::StubExt stu;
//    stu.set_lamda(ADDR(QAbstractSlider, minimum), [&ismini](){ismini = true; return -1;});
//    typedef int (*fptr)(A*,int);
//       fptr A_foo = (fptr)(&A::foo);


    typedef void (*fptr)(SectionValueLabel*,QMouseEvent *);
    fptr target = (fptr)(&SectionValueLabel::mouseReleaseEvent);
    stu.set_lamda(target, [&inThere]()->void {
        inThere = true;
        return;
    });
    LinkSectionValueLabel lsvlab;
    QMouseEvent event(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    lsvlab.mouseReleaseEvent(&event);
    EXPECT_TRUE(inThere);
}

DFM_END_NAMESPACE
