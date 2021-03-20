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
#include <QTest>
#include <QTextEdit>
#include <QTimer>
#include <QMenu>

#define private public
#include "views/fileitem.h"

TEST(FileIconItemTest,init)
{
    FileIconItem w;
    w.updateStyleSheet();
    EXPECT_NE(nullptr,w.icon);
    EXPECT_NE(nullptr,w.edit);
}

TEST(FileIconItemTest,get_icon_label)
{
    FileIconItem w;
    EXPECT_EQ(w.icon,w.getIconLabel());
}

TEST(FileIconItemTest,get_text_edit)
{
    FileIconItem w;
    EXPECT_EQ(w.edit,w.getTextEdit());
}

TEST(FileIconItemTest,opacity)
{
    FileIconItem w;
    EXPECT_EQ(nullptr,w.opacityEffect);
    EXPECT_EQ(1,w.opacity());

    w.setOpacity(0.5);
    EXPECT_NE(nullptr,w.opacityEffect);
    EXPECT_EQ(0.5,w.opacity());
}

TEST(FileIconItemTest,set_opacity)
{
    FileIconItem w;
    w.setOpacity(0.5);
    EXPECT_NE(nullptr,w.opacityEffect);
    EXPECT_EQ(0.5,w.opacity());

    w.setOpacity(1.0);
    EXPECT_EQ(nullptr,w.opacityEffect);
    EXPECT_EQ(1,w.opacity());

    w.setOpacity(1.1);
    EXPECT_EQ(nullptr,w.opacityEffect);
    EXPECT_EQ(1,w.opacity());
}

TEST(FileIconItemTest,set_maxchar_size)
{
    FileIconItem w;
    w.setMaxCharSize(10);
    EXPECT_EQ(10, w.maxCharSize());
}

TEST(FileIconItemTest,edit_textstack_currentitem)
{
    FileIconItem w;
    EXPECT_EQ(QString(""), w.editTextStackCurrentItem());

    w.pushItemToEditTextStack("test");
    EXPECT_EQ(QString("test"), w.editTextStackCurrentItem());
}

TEST(FileIconItemTest,edit_textstack_back_with_empty)
{
    FileIconItem w;
    EXPECT_EQ(QString(""), w.edit->toPlainText());
    w.editTextStackBack();
    EXPECT_EQ(QString(""), w.editTextStackCurrentItem());
}

TEST(FileIconItemTest,edit_textstack_advance_with_empty)
{
    FileIconItem w;
    EXPECT_EQ(QString(""), w.edit->toPlainText());
    w.editTextStackAdvance();
    EXPECT_EQ(QString(""), w.editTextStackCurrentItem());
    EXPECT_EQ(QString(""), w.edit->toPlainText());
}

TEST(FileIconItemTest,edit_undo_redo)
{
    FileIconItem w;
    QString str("test");
    w.edit->setPlainText(str);
    for (int i = 1; i <= str.size(); ++i)
        w.pushItemToEditTextStack(str.left(i));

    EXPECT_EQ(str,w.edit->toPlainText());
    w.editUndo();
    QString undoStr = str.left(str.size() - 1);
    EXPECT_EQ(undoStr, w.edit->toPlainText());

    w.editRedo();
    EXPECT_EQ(str, w.edit->toPlainText());
}

TEST(FileIconItemTest,show_alert_message)
{
    FileIconItem w;
    QString str("test");
    EXPECT_NO_FATAL_FAILURE(w.showAlertMessage(str));
}

TEST(FileIconItemTest,popupEditContentMenu)
{
    FileIconItem w;
    bool isExecMenu = false;
    QTimer::singleShot(1,&w,[&w,&isExecMenu](){
        for (QWidget *wid : qApp->topLevelWidgets()) {
            if (QMenu *menu = qobject_cast<QMenu *>(wid)) {
               isExecMenu = menu->parent() == w.edit;
               if (isExecMenu) {
                   menu->hide();
                   return;
               }
            }
        }
    });
    w.popupEditContentMenu();
    EXPECT_TRUE(isExecMenu);
}
