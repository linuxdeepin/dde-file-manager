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
#include <QLabel>
#include <QPushButton>
#include "stub.h"
#define private public
#define protected public
#include "dialogs/ddesktoprenamedialog.h"
#include "dialogs/private/ddesktoprenamedialog_p.h"

namespace  {
    class TestDDesktopRenameDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DDesktopRenameDialog();
            std::cout << "start TestDDesktopRenameDialog";
        }
        void TearDown() override
        {
            if (m_pTester) {
                delete m_pTester;
                m_pTester = nullptr;
            }
            std::cout << "end TestDDesktopRenameDialog";
        }
    public:
        DDesktopRenameDialog *m_pTester;
    };
}

TEST_F(TestDDesktopRenameDialog, testInit)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    DDesktopRenameDialog dlg;
    EXPECT_NE(nullptr, m_pTester);
}


TEST_F(TestDDesktopRenameDialog, testinitConnect)
{
    QAbstractButton*(*stub_getButton)(int) = [](int)->QAbstractButton*{
//        throw std::bad_cast{};
        return nullptr;
    };
    Stub stu;
    stu.set(ADDR(DDialog, getButton), stub_getButton);

    EXPECT_NO_FATAL_FAILURE(m_pTester->initConnect());
}

TEST_F(TestDDesktopRenameDialog, testGetCurrentModeIndex)
{
    std::size_t result = m_pTester->getCurrentModeIndex();
    EXPECT_EQ(result, 0);
}

TEST_F(TestDDesktopRenameDialog, testGetAddMode)
{
    DFileService::AddTextFlags result = m_pTester->getAddMode();
    EXPECT_EQ(result, DFileService::AddTextFlags::Before);
}

TEST_F(TestDDesktopRenameDialog, testGetModeOneContent)
{
    QPair<QString, QString> result = m_pTester->getModeOneContent();
    EXPECT_STREQ(result.first.toStdString().c_str(), "");
    EXPECT_STREQ(result.second.toStdString().c_str(), "");
}

TEST_F(TestDDesktopRenameDialog, testGetModeTwoContent)
{
    QPair<QString, DFileService::AddTextFlags> result = m_pTester->getModeTwoContent();
    EXPECT_STREQ(result.first.toStdString().c_str(), "");
    EXPECT_EQ(result.second, DFileService::AddTextFlags::Before);
}

TEST_F(TestDDesktopRenameDialog, testGetModeThreeContent)
{
    QPair<QString, QString> result = m_pTester->getModeThreeContent();
    EXPECT_STREQ(result.first.toStdString().c_str(), "");
    EXPECT_STREQ(result.second.toStdString().c_str(), "1");
}


TEST_F(TestDDesktopRenameDialog, testGetModeThreeContent2)
{
    bool(*stub_isEmpty)() = []()->bool{
       return true;
    };
    Stub stu;
    stu.set(ADDR(QString, isEmpty), stub_isEmpty);

    QPair<QString, QString> result = m_pTester->getModeThreeContent();
    EXPECT_STREQ(result.first.toStdString().c_str(), "");
    EXPECT_STREQ(result.second.toStdString().c_str(), "1");
}

TEST_F(TestDDesktopRenameDialog, testSetVisible)
{
    m_pTester->setVisible(false);
    bool b = m_pTester->isVisible();
    EXPECT_EQ(b, false);
}

TEST_F(TestDDesktopRenameDialog, testSetDialogTitle)
{
    m_pTester->setDialogTitle("UnitTestTitle");
    QString str = m_pTester->d_ptr->m_titleLabel->text();
    EXPECT_TRUE(str == "UnitTestTitle");
}

TEST_F(TestDDesktopRenameDialog, testOnReplaceTextChanged)
{
    QLabel label("111");
    QLineEdit line("gtest");
    QHBoxLayout lay;
    m_pTester->d_ptr->m_modeOneItemsForReplacing = std::make_tuple(&label, &line, &lay);
    m_pTester->onReplaceTextChanged();
    QString str = line.text();
    EXPECT_TRUE(str == "gtest");
}

TEST_F(TestDDesktopRenameDialog, testOnCurrentModeChanged)
{
    std::size_t index = 0;
    m_pTester->onCurrentModeChanged(index);
    EXPECT_EQ(m_pTester->d_ptr->m_currentmode, index);
}

TEST_F(TestDDesktopRenameDialog, testOnCurrentAddModeChanged)
{
    std::size_t index = 0;
    m_pTester->onCurrentAddModeChanged(index);
    EXPECT_EQ(m_pTester->d_ptr->m_flagForAdding, DFileService::AddTextFlags::Before);
}

TEST_F(TestDDesktopRenameDialog, testOnCurrentAddModeChanged2)
{
    std::size_t index = 1;
    m_pTester->onCurrentAddModeChanged(index);
    EXPECT_EQ(m_pTester->d_ptr->m_flagForAdding, DFileService::AddTextFlags::After);
}

TEST_F(TestDDesktopRenameDialog, testSetRenameButtonStatus)
{
    m_pTester->setRenameButtonStatus(false);
    bool b = m_pTester->getButton(1)->isVisible();
    EXPECT_EQ(b, false);
}

TEST_F(TestDDesktopRenameDialog, testOnContentChangedForFinding)
{
    m_pTester->onContentChangedForFinding("123");
    EXPECT_EQ(m_pTester->d_func()->m_currentEnabled[0], true);
}

TEST_F(TestDDesktopRenameDialog, testOnContentChangedForFinding2)
{
    m_pTester->onContentChangedForFinding("");
    EXPECT_EQ(m_pTester->d_func()->m_currentEnabled[0], false);
}

TEST_F(TestDDesktopRenameDialog, testOnContentChangedForAdding)
{
    m_pTester->onContentChangedForAdding("123");
    bool b = m_pTester->d_func()->m_currentEnabled[0];
    EXPECT_EQ(b, false);
}

TEST_F(TestDDesktopRenameDialog, testOnContentChangedForAdding2)
{
    m_pTester->onContentChangedForAdding("");
    EXPECT_EQ(m_pTester->d_func()->m_currentEnabled[0], false);
}

TEST_F(TestDDesktopRenameDialog, testOnContentChangedForCustomzedSN)
{
    m_pTester->onContentChangedForCustomzedSN("123");
}

TEST_F(TestDDesktopRenameDialog, testOnContentChangedForCustomzedSN2)
{
    m_pTester->onContentChangedForCustomzedSN("sdgfdjgh234534523");
    QString str = std::get<1>(m_pTester->d_func()->m_modeThreeItemsForSNNumber)->text();
    EXPECT_TRUE(str == "1");
}

TEST_F(TestDDesktopRenameDialog, testOnContentChangedForCustomzedSN3)
{
    m_pTester->onContentChangedForCustomzedSN("1233247859347689720937534980674986973534");
    QString str = std::get<1>(m_pTester->d_func()->m_modeThreeItemsForSNNumber)->text();
    EXPECT_TRUE(str == "1");
}

TEST_F(TestDDesktopRenameDialog, testOnVisibleChanged)
{
    m_pTester->d_ptr->m_currentmode = 1;
    EXPECT_NO_FATAL_FAILURE(m_pTester->onVisibleChanged(true));
}

TEST_F(TestDDesktopRenameDialog, testOnVisibleChanged2)
{
    m_pTester->d_ptr->m_currentmode = 2;
    EXPECT_NO_FATAL_FAILURE(m_pTester->onVisibleChanged(true));
}

TEST_F(TestDDesktopRenameDialog, testOnVisibleChanged3)
{
    m_pTester->d_ptr->m_currentmode = 3;
    EXPECT_NO_FATAL_FAILURE(m_pTester->onVisibleChanged(true));
}

namespace  {
    class TestDDesktopRenameDialogPrivate : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DDesktopRenameDialogPrivate(nullptr);
            std::cout << "start TestDDesktopRenameDialogPrivate";
        }
        void TearDown() override
        {
            if (m_pTester) {
                delete m_pTester;
                m_pTester = nullptr;
            }
            std::cout << "end TestDDesktopRenameDialogPrivate";
        }
    public:
        DDesktopRenameDialogPrivate *m_pTester;
    };
}

TEST_F(TestDDesktopRenameDialogPrivate, testUpdateLineEditText)
{
    QLineEdit lineEdit("a:bc");
    QString defaultValue("abc");
    m_pTester->updateLineEditText(&lineEdit, defaultValue);
    QString str = lineEdit.text();
    EXPECT_TRUE(str == "abc");
}
