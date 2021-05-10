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

#include <QComboBox>
#include <gtest/gtest.h>

#include "dfilestatisticsjob.h"
#include "stub.h"
#include <dlistview.h>

#define private public
#define protected public
#include "views/dfilemanagerwindow.h"
#include "dialogs/connecttoserverdialog.h"

namespace  {
    class TestConnectToServerDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
            Stub stu;
            stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

            bool(*stu_openNewTab)(DUrl fileUrl) = [](DUrl fileUrl)->bool{
                Q_UNUSED(fileUrl);
                return true;
            };
            Stub stu2;
            stu2.set(ADDR(DFileManagerWindow, openNewTab), stu_openNewTab);

            void(*stu_initTitleBar)() = [](){};
            Stub stu3;
            stu3.set(ADDR(DFileManagerWindow, initTitleBar), stu_initTitleBar);

            m_pMainwindow = new DFileManagerWindow();
            m_pTester = new ConnectToServerDialog(m_pMainwindow);
            std::cout << "start TestConnectToServerDialog";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete m_pTester;
            m_pTester = nullptr;
            if(m_pMainwindow)
                delete m_pMainwindow;
            m_pMainwindow = nullptr;
            std::cout << "end TestConnectToServerDialog";
        }
    public:
        ConnectToServerDialog *m_pTester;
        DFileManagerWindow* m_pMainwindow;
    };
}
#ifndef __arm__
TEST_F(TestConnectToServerDialog, testInit)
{
    EXPECT_NE(nullptr, m_pTester);
}

TEST_F(TestConnectToServerDialog, testInitUI)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    ConnectToServerDialog dlg;
    QString str = dlg.windowTitle();
    EXPECT_TRUE(str == "Connect to Server");
}

TEST_F(TestConnectToServerDialog, testOnButtonClicked)
{
    int index = 1;
    m_pTester->onButtonClicked(index);
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "");
}

TEST_F(TestConnectToServerDialog, testOnButtonClicked2)
{
    QString(*stub_currentText)() = []()->QString{
        return "GTest";
    };
    Stub stu;
    stu.set(ADDR(QComboBox, currentText), stub_currentText);

    bool(*stu_cd)() = []()->bool{
            return false;
    };
    Stub stu2;
    stu2.set(ADDR(DFileManagerWindow, cd), stu_cd);

    int index = 1;
    m_pTester->onButtonClicked(index);
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "GTest");
}

TEST_F(TestConnectToServerDialog, testOnAddButtonClicked)
{
    m_pTester->m_serverComboBox->setEditText("123");
    m_pTester->onAddButtonClicked();
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "123");
}

TEST_F(TestConnectToServerDialog, testOnAddButtonClicked2)
{
    bool(*stub_addItem)(const QVariant&) = [](const QVariant&)->bool{
        return false;
    };

    Stub stu;
    stu.set(ADDR(DListView, addItem), stub_addItem);

    m_pTester->m_serverComboBox->setEditText("789");
    m_pTester->onAddButtonClicked();
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "789");
}

TEST_F(TestConnectToServerDialog, testOnAddButtonClicked3)
{
    m_pTester->m_serverComboBox->setEditText("456");
    m_pTester->onAddButtonClicked();
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "456");
}

TEST_F(TestConnectToServerDialog, testOnDelButtonClicked)
{
    m_pTester->m_serverComboBox->setEditText("123");
    m_pTester->onDelButtonClicked();
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "");
}

TEST_F(TestConnectToServerDialog, testOnDelButtonClicked2)
{
    bool(*stub_removeItem)(int) = [](int)->bool{
        return false;
    };

    Stub stu;
    stu.set(ADDR(DListView, removeItem), stub_removeItem);

    m_pTester->m_serverComboBox->setEditText("456");
    m_pTester->onDelButtonClicked();
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "456");
}

TEST_F(TestConnectToServerDialog, testOnDelButtonClicked3)
{
    bool(*stub_isValid)() = []()->bool{
        return true;
    };

    Stub stu;
    stu.set(ADDR(QModelIndex, isValid), stub_isValid);

    m_pTester->m_serverComboBox->setEditText("456");
    m_pTester->onDelButtonClicked();
    QString str = m_pTester->m_serverComboBox->currentText();
    EXPECT_TRUE(str == "");
}
#endif
