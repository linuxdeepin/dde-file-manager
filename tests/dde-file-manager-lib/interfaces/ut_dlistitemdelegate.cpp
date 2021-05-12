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

#include "views/fileviewhelper.h"
#include "interfaces/dfmviewmanager.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QIcon>
#include <QPainter>
#include <QTextDocument>
#include <QEvent>

#define protected public
#define private public
#include "interfaces/dlistitemdelegate.h"
#include "views/dfileview.h"
#include "testhelper.h"

#define FreePtr(x) if(x){delete x;x=nullptr;}

namespace  {
class TestDListItemDelegate : public testing::Test
{
public:
    DListItemDelegate *dlistIdl;
    DFileView *dfileview;
    void SetUp() override
    {
        dfileview = new DFileView();
        dfileview->initDelegate();
        dfileview->increaseIcon();
        dfileview->decreaseIcon();
        dfileview ->setIconSizeBySizeIndex(0);
        dfileview->fileViewHelper()->model()   ;
        dlistIdl = new DListItemDelegate(dfileview->fileViewHelper());
        std::cout << "start TestDListItemDelegate";
    }
    void TearDown() override
    {
        delete dlistIdl;
        dlistIdl = nullptr;
        delete dfileview;
        dfileview = nullptr;
        TestHelper::runInLoop([](){}, 10);
        std::cout << "end TestDListItemDelegate";
    }
};
}

TEST_F(TestDListItemDelegate, test_paint)
{
    QPainter *painter = new QPainter();
    QStyleOptionViewItem option;
    QModelIndex index = QModelIndex();
    dlistIdl->paint(painter, option, index);
    delete painter;
    painter = nullptr;
}

TEST_F(TestDListItemDelegate, test_paint2)
{
    QPainter *painter = new QPainter();
    QStyleOptionViewItem option;
    option.state = QStyle::State_Selected;
    option.showDecorationSelected = true;
    QModelIndex index = QModelIndex();
    dlistIdl->paint(painter, option, index);
    delete painter;
    painter = nullptr;
}

TEST_F(TestDListItemDelegate, test_sizeHint)
{
    QStyleOptionViewItem viewItem;
    QModelIndex index = QModelIndex();
    dlistIdl->sizeHint(viewItem, index);
}

TEST_F(TestDListItemDelegate, test_updateEditorGeometry)
{
    QWidget *editor = new QWidget();
    QStyleOptionViewItem option;
    QModelIndex index = QModelIndex();
    dlistIdl->updateEditorGeometry(editor, option, index);
    editor->deleteLater();
}

TEST_F(TestDListItemDelegate, test_setEditorData)
{
    QWidget *editor = new QWidget();
    QModelIndex index = QModelIndex();
    dlistIdl->setEditorData(editor,  index);
    editor->deleteLater();
}

TEST_F(TestDListItemDelegate, test_updateItemSizeHint)
{
    dlistIdl->updateItemSizeHint();
}

TEST_F(TestDListItemDelegate, test_eventFilter)
{
    QObject *object = new QObject();
    QEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    dlistIdl->eventFilter(object, event);
    FreePtr(object);
    FreePtr(event);
}


TEST_F(TestDListItemDelegate, test_helpEvent)
{
    QHelpEvent *event = new QHelpEvent(QEvent::ToolTip, QPoint(10, 10), QPoint(10, 10));
    QAbstractItemView *view = nullptr;
    QStyleOptionViewItem option;
    QModelIndex index = QModelIndex();
    dlistIdl->helpEvent(event, view, option, index);
    FreePtr(event);
}

