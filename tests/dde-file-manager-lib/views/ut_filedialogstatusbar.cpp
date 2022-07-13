/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QApplication>

#define private public
#include "views/filedialogstatusbar.h"

TEST(FileDialogStatusBarTest,init)
{
    FileDialogStatusBar w;
    EXPECT_NE(nullptr,w.m_contentLayout);
    EXPECT_NE(nullptr,w.m_titleLabel);
    EXPECT_NE(nullptr,w.m_fileNameLabel);
    EXPECT_NE(nullptr,w.m_filtersLabel);
    EXPECT_NE(nullptr,w.m_fileNameEdit);
    EXPECT_NE(nullptr,w.m_filtersComboBox);
    EXPECT_NE(nullptr,w.m_acceptButton);
    EXPECT_NE(nullptr,w.m_rejectButton);
}

TEST(FileDialogStatusBarTest,set_mode)
{
    FileDialogStatusBar w;
    w.setMode(FileDialogStatusBar::Unknow);

    w.setMode(FileDialogStatusBar::Open);
    EXPECT_EQ(FileDialogStatusBar::Open,w.m_mode);
}

TEST(FileDialogStatusBarTest,set_comboxitems_with_empty)
{
    FileDialogStatusBar w;
    EXPECT_EQ(w.m_fileNameEdit,w.lineEdit());
    EXPECT_EQ(w.m_filtersComboBox,w.comboBox());
    EXPECT_EQ(w.m_acceptButton,w.acceptButton());
    EXPECT_EQ(w.m_rejectButton,w.rejectButton());
}

TEST(FileDialogStatusBarTest,get_widget)
{
    FileDialogStatusBar w;
    w.setMode(FileDialogStatusBar::Open);
    w.setComBoxItems({"test"});
    EXPECT_EQ(1,w.m_filtersComboBox->count());
    EXPECT_EQ(false,w.m_filtersLabel->isHidden());
    EXPECT_EQ(false,w.m_filtersComboBox->isHidden());
}

TEST(FileDialogStatusBarTest,set_comboxitems_with_open)
{
    FileDialogStatusBar w;
    w.setMode(FileDialogStatusBar::Open);
    w.setComBoxItems({"test"});
    EXPECT_EQ(1,w.m_filtersComboBox->count());
    EXPECT_EQ(false,w.m_filtersLabel->isHidden());
    EXPECT_EQ(false,w.m_filtersComboBox->isHidden());
}


TEST(FileDialogStatusBarTest,set_comboxitems_with_open_empty)
{
    FileDialogStatusBar w;
    w.setMode(FileDialogStatusBar::Open);
    w.setComBoxItems({});
    EXPECT_EQ(0,w.m_filtersComboBox->count());
    EXPECT_EQ(true,w.m_filtersLabel->isHidden());
    EXPECT_EQ(true,w.m_filtersComboBox->isHidden());
}

TEST(FileDialogStatusBarTest,add_line_edit)
{
    FileDialogStatusBar w;
    EXPECT_EQ(0,w.m_customLineEditList.size());
    QLabel l;
    QLineEdit le;
    w.addLineEdit(&l,&le);
    EXPECT_EQ(1,w.m_customLineEditList.size());
}

TEST(FileDialogStatusBarTest,get_line_edit_value)
{
    FileDialogStatusBar w;
    QString str("test");
    EXPECT_EQ(QString(""),w.getLineEditValue(str));

    QLabel l(str);
    QLineEdit le;
    le.setText(str);
    w.addLineEdit(&l,&le);
    EXPECT_EQ(str,w.getLineEditValue(str));
}

TEST(FileDialogStatusBarTest,all_line_edits_value)
{
    FileDialogStatusBar w;
    QString str("test");
    QLabel l(str);
    QLineEdit le;
    le.setText(str);
    w.addLineEdit(&l,&le);

    auto valMap = w.allLineEditsValue();
    ASSERT_EQ(1,valMap.size());
    ASSERT_EQ(true,valMap.contains(str));
    EXPECT_EQ(str,valMap.value(str).toString());
}

TEST(FileDialogStatusBarTest,add_combobox)
{
    FileDialogStatusBar w;
    EXPECT_EQ(0,w.m_customComboBoxList.size());
    QLabel l;
    QComboBox box;
    w.addComboBox(&l,&box);
    EXPECT_EQ(1,w.m_customComboBoxList.size());
}

TEST(FileDialogStatusBarTest,get_combobox_value)
{
    FileDialogStatusBar w;
    QString str("test");
    EXPECT_EQ(QString(""),w.getComboBoxValue(str));

    QLabel l(str);
    QComboBox box;
    box.addItem(str);
    box.setCurrentText(str);
    w.addComboBox(&l,&box);
    EXPECT_EQ(str,w.getComboBoxValue(str));
}

TEST(FileDialogStatusBarTest,all_combobox_value)
{
    FileDialogStatusBar w;
    QString str("test");
    QLabel l(str);
    QComboBox box;
    box.addItem(str);
    box.setCurrentText(str);
    w.addComboBox(&l,&box);

    auto valMap = w.allComboBoxsValue();
    ASSERT_EQ(1,valMap.size());
    ASSERT_EQ(true,valMap.contains(str));
    EXPECT_EQ(str,valMap.value(str).toString());
}

TEST(FileDialogStatusBarTest,show)
{
    FileDialogStatusBar w;
    w.show();
    EXPECT_EQ(true,w.isVisible());

    w.hide();
    EXPECT_EQ(false,w.isVisible());
}

TEST(FileDialogStatusBarTest,begin_add_customwidget)
{
    FileDialogStatusBar w;
    QString str("test");
    QLabel l(str);
    QComboBox box;
    box.addItem(str);
    box.setCurrentText(str);
    w.addComboBox(&l,&box);

    QString str2("test2");
    QLabel ll(str2);
    QLineEdit le;
    le.setText(str2);
    w.addLineEdit(&ll,&le);

    w.beginAddCustomWidget();
    EXPECT_EQ(0,w.m_customLineEditList.size());
    EXPECT_EQ(0,w.m_customComboBoxList.size());
    w.endAddCustomWidget();
}

TEST(FileDialogStatusBarTest,updateLayout_delete_layout)
{
    FileDialogStatusBar w;
    QHBoxLayout *lay = new QHBoxLayout;
    w.m_contentLayout->addLayout(lay);
    w.m_mode = FileDialogStatusBar::Save;

    w.updateLayout();
    for (int i = 0; i < w.m_contentLayout->count(); ++i)
        EXPECT_FALSE(w.m_contentLayout->itemAt(i) == lay);
}

TEST(FileDialogStatusBarTest,updateLayout_save)
{
    FileDialogStatusBar w;
    w.m_mode = FileDialogStatusBar::Save;

    w.updateLayout();
    bool fileNameLabel = false;
    bool fileNameEdit = false;
    bool acceptButton = false;
    bool rejectButton = false;

    for (int i = 0; i < w.m_contentLayout->count(); ++i) {
        auto wid = w.m_contentLayout->itemAt(i)->widget();
        if (wid == w.m_fileNameEdit)
            fileNameEdit = true;
        else if (wid == w.m_fileNameLabel)
            fileNameLabel = true;
        else if (wid == (QWidget *)w.m_acceptButton)
            acceptButton = true;
        else if (wid == (QWidget *)w.m_rejectButton)
            rejectButton = true;
    }
    EXPECT_TRUE(fileNameLabel);
    EXPECT_TRUE(fileNameEdit);
    EXPECT_TRUE(acceptButton);
    EXPECT_TRUE(rejectButton);
}

TEST(FileDialogStatusBarTest,updateLayout_save_two)
{
    FileDialogStatusBar w;
    w.m_mode = FileDialogStatusBar::Save;
    w.show();
    w.setComBoxItems({"1","2"});
    w.updateLayout();

    EXPECT_TRUE(w.m_filtersLabel->isVisible());
    EXPECT_TRUE(w.m_filtersComboBox->isVisible());
}
