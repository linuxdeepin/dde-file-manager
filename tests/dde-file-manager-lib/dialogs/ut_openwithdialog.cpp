/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include <QIcon>
#include <QEvent>
#include <QRect>
#include <QPaintEvent>
#include <QFileDialog>

#include "stub.h"
#include "dfmglobal.h"
#include "interfaces/dfileservices.h"

#define protected public
#define private public
#include "dialogs/openwithdialog.h"

namespace  {
    class TestOpenWithDialogListItem : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new OpenWithDialogListItem(QIcon(), "OpenWithDialogListItem");
            std::cout << "start TestOpenWithDialogListItem";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestOpenWithDialogListItem";
        }
    public:
        OpenWithDialogListItem  *m_pTester;
    };
}

TEST_F(TestOpenWithDialogListItem, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestOpenWithDialogListItem, testSetCheckedTrue)
{
    m_pTester->setChecked(true);
    EXPECT_NE(m_pTester->m_checkButton, nullptr);
}

TEST_F(TestOpenWithDialogListItem, testSetCheckedFalse)
{
    m_pTester->setChecked(false);
    EXPECT_NE(m_pTester->m_checkButton, nullptr);
}

TEST_F(TestOpenWithDialogListItem, testText)
{
    QString str = m_pTester->text();
    EXPECT_TRUE(str == "OpenWithDialogListItem");
}

TEST_F(TestOpenWithDialogListItem, testEnterEvent)
{
    QEvent event(QEvent::KeyPress);
    EXPECT_NO_FATAL_FAILURE(m_pTester->enterEvent(&event));
}

TEST_F(TestOpenWithDialogListItem, testLeaveEvent)
{
    QEvent event(QEvent::KeyPress);
    EXPECT_NO_FATAL_FAILURE(m_pTester->leaveEvent(&event));
}

TEST_F(TestOpenWithDialogListItem, testPaintEvent)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    EXPECT_NO_FATAL_FAILURE(m_pTester->paintEvent(&event));
}

namespace  {
    class TestOpenWithDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            DUrl url("file:///test3");
            m_pTester = new OpenWithDialog(url);
            std::cout << "start TestOpenWithDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestOpenWithDialog";
        }
    public:
        OpenWithDialog  *m_pTester;
    };
}

TEST_F(TestOpenWithDialog, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestOpenWithDialog, testInit2)
{
    QList<DUrl> urllist;
    urllist << DUrl("file:///test1") << DUrl("file:///test2");
    OpenWithDialog dlg(urllist);
//    dlg.show();
    int count = dlg.m_urllist.count();
    EXPECT_EQ(count, 2);
}

TEST_F(TestOpenWithDialog, testInit3)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    DUrl url("file:///test3");
    OpenWithDialog  dlg(url);
    QString str = dlg.m_url.toString();
    EXPECT_TRUE(str == "file:///test3");
}

TEST_F(TestOpenWithDialog, testOpenFileByApp)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->openFileByApp());
}

TEST_F(TestOpenWithDialog, testOpenFileByApp2)
{
    OpenWithDialogListItem item(QIcon::fromTheme("media-optical"), "utest");
    m_pTester->m_checkedItem = &item;
    m_pTester->openFileByApp();
    EXPECT_NE(m_pTester->m_checkedItem, nullptr);
}

TEST_F(TestOpenWithDialog, testOpenFileByApp3)
{
    OpenWithDialogListItem item(QIcon::fromTheme("media-optical"), "utest");
    m_pTester->m_checkedItem = &item;
    m_pTester->m_urllist.push_back(DUrl("file:///test1"));

    bool(*stub_openFileByApp)(const QObject *, const QString &, const DUrl &) = [](const QObject *, const QString &, const DUrl &)->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFileService, openFileByApp), stub_openFileByApp);

    m_pTester->openFileByApp();
    EXPECT_NE(m_pTester->m_checkedItem, nullptr);
}

TEST_F(TestOpenWithDialog, testOpenFileByApp4)
{
    OpenWithDialogListItem item(QIcon::fromTheme("media-optical"), "utest");
    m_pTester->m_checkedItem = &item;
    m_pTester->m_urllist.push_back(DUrl("file:///test1"));

    m_pTester->m_url = DUrl("");

    bool(*stub_openFileByApp)(const QObject *, const QString &, const DUrl &) = [](const QObject *, const QString &, const DUrl &)->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFileService, openFileByApp), stub_openFileByApp);

    m_pTester->openFileByApp();
    EXPECT_NE(m_pTester->m_checkedItem, nullptr);
}

TEST_F(TestOpenWithDialog, testOpenFileByApp5)
{
    OpenWithDialogListItem item(QIcon::fromTheme("media-optical"), "utest");
    m_pTester->m_checkedItem = &item;
    m_pTester->m_urllist.push_back(DUrl("file:///test1"));
    m_pTester->m_urllist.push_back(DUrl("file:///test2"));

    m_pTester->m_url = DUrl("");
    bool(*stub_openFilesByApp)(const QObject *, const QString &, const QList<DUrl> &, const bool) = [](const QObject *, const QString &, const QList<DUrl> &, const bool)->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFileService, openFilesByApp), stub_openFilesByApp);

    m_pTester->openFileByApp();
    EXPECT_NE(m_pTester->m_checkedItem, nullptr);
}

TEST_F(TestOpenWithDialog, testUseOtherApplication)
{
    QString(*stu_getOpenFileName)(QWidget*, const QString &, const QString &, const QString &, QString *, QFileDialog::Options)
            = [](QWidget*, const QString &, const QString &, const QString &, QString *, QFileDialog::Options)->QString{
        return "/usr/bin/deepin-movie.desktop";
    };
    Stub stu;
    stu.set(ADDR(QFileDialog, getOpenFileName), stu_getOpenFileName);

    EXPECT_NO_FATAL_FAILURE(m_pTester->useOtherApplication());
}

TEST_F(TestOpenWithDialog, testUseOtherApplication2)
{
    QString(*stu_getOpenFileName)(QWidget*, const QString &, const QString &, const QString &, QString *, QFileDialog::Options)
            = [](QWidget*, const QString &, const QString &, const QString &, QString *, QFileDialog::Options)->QString{
        return "/test1";
    };
    Stub stu;
    stu.set(ADDR(QFileDialog, getOpenFileName), stu_getOpenFileName);

//    bool isExecutable() const;
    bool(*stu_isExecutable)() = []()->bool{
        return true;
    };
    Stub stu2;
    stu2.set(ADDR(QFileInfo, isExecutable), stu_isExecutable);

    EXPECT_NO_FATAL_FAILURE(m_pTester->useOtherApplication());
}

TEST_F(TestOpenWithDialog, testCreateItem)
{
    OpenWithDialogListItem *item = m_pTester->createItem(QIcon::fromTheme("application-x-desktop"), "appName", "/appname");
    EXPECT_NE(item, nullptr);
}

TEST_F(TestOpenWithDialog, testShowEvent)
{
    QShowEvent event;
    m_pTester->showEvent(&event);
    int width = m_pTester->m_recommandLayout->parentWidget()->width();
    EXPECT_NE(width, 0);
}

TEST_F(TestOpenWithDialog, testEventFilter)
{
    OpenWithDialogListItem item(QIcon(), "testItem");
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->eventFilter(&item, &event));
}
