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

#include "dfmevent.h"
#include "dfmapplication.h"
#include "shutil/fileutils.h"
#include "stub.h"
#include "stub-ext/stubext.h"
#include <QComboBox>

#define private public
#define protected public
#include <views/dstatusbar.h>


TEST(DstatusbarTest,init)
{
    DStatusBar w;

    EXPECT_NE(nullptr, w.m_scaleSlider);
    EXPECT_TRUE(nullptr == w.m_rejectButton);
    EXPECT_TRUE(nullptr == w.m_lineEdit);
    EXPECT_TRUE(nullptr == w.m_comboBox);
//    EXPECT_TRUE(nullptr == w.m_lineEditLabel);
//    EXPECT_TRUE(nullptr == w.m_comboBoxLabel);
}

TEST(DstatusbarTest, mode)
{
    DStatusBar w;
    w.m_mode = DStatusBar::Mode::Normal;
    auto temp1 = w.mode() == DStatusBar::Mode::Normal;
    EXPECT_TRUE(temp1);

    w.m_mode = DStatusBar::Mode::DialogOpen;
    auto temp2 = w.mode() == DStatusBar::Mode::DialogOpen;
    EXPECT_TRUE(temp2);

    w.m_mode = DStatusBar::Mode::DialogSave;
    auto temp3 = w.mode() == DStatusBar::Mode::DialogSave;
    EXPECT_TRUE(temp3);
}

TEST(DstatusbarTest, setMode)
{
    DStatusBar w;

    w.setMode(DStatusBar::Mode::DialogOpen);
    auto tempOpen = w.mode() == DStatusBar::Mode::DialogOpen;
    EXPECT_TRUE(tempOpen);

    w.setMode(DStatusBar::Mode::DialogSave);
    auto tempSave = w.mode() == DStatusBar::Mode::DialogSave;
    EXPECT_TRUE(tempSave);

    w.m_label = nullptr;
    w.setMode(DStatusBar::Mode::Normal);
    auto temp1 = w.mode() == DStatusBar::Mode::Normal;
    EXPECT_TRUE(temp1);
}

TEST(DstatusbarTest, setComBoxItems)
{
    DStatusBar w;

    w.setMode(DStatusBar::Mode::Normal);
    EXPECT_TRUE(nullptr == w.m_comboBox);

    w.setMode(DStatusBar::Mode::DialogSave);
    QStringList temp1{"aa" , "bb" , "cc" , "dd"};
    w.setComBoxItems(temp1);
    auto temp = temp1.size() == w.m_comboBox->count();
    EXPECT_TRUE(temp);

}

TEST(DstatusbarTest, scalingSlider)
{
    DStatusBar w;
    EXPECT_TRUE(w.scalingSlider() == w.m_scaleSlider);
}

TEST(DstatusbarTest, acceptButton)
{
    DStatusBar w;
    EXPECT_TRUE(w.acceptButton() == w.m_acceptButton);
    w.m_mode = DStatusBar::Mode::DialogSave;
    EXPECT_TRUE(w.acceptButton() == w.m_acceptButton);
}

TEST(DstatusbarTest, rejectButton)
{
    DStatusBar w;
    EXPECT_TRUE(w.rejectButton() == w.m_rejectButton);
    w.m_mode = DStatusBar::Mode::DialogSave;
    EXPECT_TRUE(w.rejectButton() == w.m_rejectButton);
}

TEST(DstatusbarTest, lineEdit)
{
    DStatusBar w;
    EXPECT_TRUE(w.lineEdit() == w.m_lineEdit);
    w.m_mode = DStatusBar::Mode::DialogSave;
    EXPECT_TRUE(w.lineEdit() == w.m_lineEdit);
}

TEST(DstatusbarTest, comboBox){
    DStatusBar w;
    EXPECT_TRUE(w.comboBox() == w.m_comboBox);
    w.m_mode = DStatusBar::Mode::DialogSave;
    EXPECT_TRUE(w.comboBox() == w.m_comboBox);
}

TEST(DstatusbarTest, sizeHint)
{

    QWidget wgt;
    DStatusBar w;
    auto layot_size = w.layout()->sizeHint();
    auto actualValue = w.sizeHint().height();
    auto expectedValue = qMax(25, layot_size.height());
    EXPECT_TRUE(actualValue == expectedValue);
}

TEST(DFMElidLabelTest, setText)
{
    DFMElidLabel ww;
    ww.setText("aa");
    EXPECT_TRUE(ww.m_text == "aa");
}

TEST(DstatusbarTest, computerSize)
{
    int pid = getpid();
    const QString path = QString("file:///proc/%0/exe").arg(pid);
    DStatusBar w;
    DUrlList list = {DUrl(path)};
    EXPECT_NE(0,w.computerSize(list));
}

TEST(DstatusbarTest, computerFolderContains)
{
    int pid = getpid();
    const QString path = QString("file:///proc/%0").arg(pid);
    DStatusBar w;
    w.initJobConnection();
    DUrlList list = {DUrl(path)};
    EXPECT_NE(0,w.computerFolderContains(list));
}

TEST(DstatusbarTest, setLoadingIncatorVisible)
{
    DStatusBar w;
    w.setLoadingIncatorVisible(false,"ok");
    EXPECT_TRUE(w.m_label->text().isEmpty());

    w.setLoadingIncatorVisible(true,"ok");
    EXPECT_EQ(QString("ok"),w.m_label->text());
}

TEST(DstatusbarTest, updateStatusMessage)
{
    DStatusBar w;
    w.updateStatusMessage();
    EXPECT_TRUE(w.m_label->text().isEmpty());

    w.m_folderCount = 1;
    w.m_folderContains = 1;
    w.updateStatusMessage();
    EXPECT_FALSE(w.m_label->text().isEmpty());

    w.m_fileCount = 1;
    w.updateStatusMessage();
    EXPECT_FALSE(w.m_label->text().isEmpty());
}

TEST(DstatusbarTest, itemSelected_with_empty)
{
    DStatusBar w;
    DFMEvent event(&w);
    event.setWindowId(0);
    event.setData({});
    w.itemSelected(event, 1);

    EXPECT_NE(nullptr,w.m_fileStatisticsJob);
}

TEST(DstatusbarTest, itemSelected_with_one)
{
    DStatusBar w;
    DFMEvent event(&w);
    event.setWindowId(0);
    event.setData({});
    Stub stub;
    auto utFileUrlList = (DUrlList(*)())([]{
        DUrlList tp;
        tp.append(DUrl("/home/utTest/one"));
        return tp;
    });
    stub.set(ADDR(DFMEvent, fileUrlList), utFileUrlList);
    w.itemSelected(event, 1);

    EXPECT_NE(nullptr,w.m_fileStatisticsJob);
}

TEST(DstatusbarTest, itemSelected_get_attribute)
{
    DStatusBar w;
    DFMEvent event(&w);
    event.setWindowId(0);
    event.setData({});

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(DFMApplication,genericAttribute),[](DFMApplication::GenericAttribute ga){
        Q_UNUSED(ga);
        return true;
    });

    w.itemSelected(event, 1);

    EXPECT_NE(nullptr,w.m_fileStatisticsJob);
}

TEST(DstatusbarTest, itemSelected_with_mtp_access)
{
    DStatusBar w;
    DFMEvent event(&w);
    event.setWindowId(0);
    event.setData({});

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(DFMApplication,genericAttribute),[](DFMApplication::GenericAttribute ga){
        Q_UNUSED(ga);
        return false;
    });

    Stub stub2;
    bool (*contains_ut)(const QString&, Qt::CaseSensitivity) = [](const QString&, Qt::CaseSensitivity){
        return true;
    };
    using ut_over = bool(QString::*)(const QString&, Qt::CaseSensitivity)const;
    stub2.set((ut_over)&QString::contains, contains_ut);

    w.itemSelected(event, 1);

    EXPECT_EQ(nullptr,w.m_fileStatisticsJob);
}

static bool inThere = false;
//TEST(DstatusbarTest, handdle_computer_folder_containsFinished)
//{
//    Stub stub;
//    auto utFun = (void(*)())([]{
//        inThere = true;
//    });
//    stub.set(ADDR(DStatusBar, updateStatusMessage), utFun);
//    DStatusBar w;

//    w.handdleComputerFolderContainsFinished();
//    EXPECT_TRUE(inThere);
//}

//TEST(DstatusbarTest, handdle_computer_fileSize_finished)
//{
//    Stub stub;
//    auto utFun = (void(*)())([]{
//        inThere = true;
//    });
//    stub.set(ADDR(DStatusBar, updateStatusMessage), utFun);

//    DStatusBar w;
//    w.handdleComputerFileSizeFinished();
//    EXPECT_TRUE(inThere);
//}

TEST(DstatusbarTest, item_counted)
{
    DFMEvent event(nullptr);

    DStatusBar w;
    w.itemCounted(event,1);
    auto tt = w.m_label->text();
    EXPECT_TRUE("1 item" == w.m_label->text());
    w.itemCounted(event,2);
    auto t2 =w.m_label->text();
    EXPECT_TRUE("2 items" == w.m_label->text());
}

TEST(DFMElidLabel, resize_event)
{
    Stub stub;
    inThere = false;
    auto utFun = (void(*)())([]{
        inThere = true;
    });
    stub.set(ADDR(DFMElidLabel, setElidText), utFun);
    QResizeEvent event(QSize(10, 10), QSize(20, 20));
    DFMElidLabel w;
    w.resizeEvent(&event);
    EXPECT_TRUE(inThere);
}




























