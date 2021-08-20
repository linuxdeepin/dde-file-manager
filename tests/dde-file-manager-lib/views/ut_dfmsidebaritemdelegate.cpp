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
#include "app/define.h"
#include "dfileservices.h"

#define private public
#include <views/dfmsidebarview.h>
#include <models/dfmsidebarmodel.h>
#include <interfaces/dfmsidebaritem.h>
#include <views/dfmsidebaritemdelegate.h>

#include <QDir>
#include <QPainter>
#include <QStandardPaths>

DFM_USE_NAMESPACE
namespace  {
    class DFMSideBarItemDelegateTest:public testing::Test
    {
    public:
        DFMSideBarView m_sidebarView;
        DFMSideBarItemDelegate *m_sideBarItemDelegate{ nullptr };

        virtual void SetUp() override
        {
            m_sidebarView.setItemDelegate(new DFMSideBarItemDelegate(&m_sidebarView));
            m_sideBarItemDelegate = dynamic_cast<DFMSideBarItemDelegate *>(m_sidebarView.itemDelegate());
        }

        virtual void TearDown() override
        {
        }
    };
}

static bool inThere = false;
static DFMSideBarModel tempModel;
TEST_F(DFMSideBarItemDelegateTest, paintIndexValidTest){
    Stub stub;
    auto utData = static_cast<QVariant(*)(int)>([](int){
        QVariant temp(DFMSideBarItem::Separator);
        return temp;
    });
    auto utPaintSeparator = static_cast<void(*)(QPainter *
                                                , const QStyleOptionViewItem &)>([](QPainter *painter
                                                                                 , const QStyleOptionViewItem &option){
            Q_UNUSED(painter);
            Q_UNUSED(option);
            inThere = true;
    });

    QPainter painter;
    QStyleOptionViewItem option;
    QModelIndex index;
    inThere = false;
    stub.set(ADDR(QModelIndex, data), utData);
    stub.set(ADDR(DFMSideBarItemDelegate, paintSeparator), utPaintSeparator);
    m_sideBarItemDelegate->paint(&painter, option, index);
    EXPECT_TRUE(inThere);
}

//TEST_F(DFMSideBarItemDelegateTest, paintIndexInValidTest){
//    Stub stub;
//    auto utPaint = static_cast<void(*)(QPainter *
//                                       , const QStyleOptionViewItem &
//                                       , const QModelIndex &)>([](QPainter *painter
//                                                               , const QStyleOptionViewItem &option
//                                                               , const QModelIndex &index){
//            Q_UNUSED(painter);
//            Q_UNUSED(option);
//            Q_UNUSED(index);
//            inThere = true;
//    });

//    QPainter painter;
//    QStyleOptionViewItem option;
//    QModelIndex index;
//    inThere = false;
//    stub.set(ADDR(DStyledItemDelegate, paint), utPaint);
//    m_sideBarItemDelegate->paint(&painter, option, index);
//    EXPECT_TRUE(inThere);
//}

/*TEST_F(DFMSideBarItemDelegateTest, sizeHintTest){
    QStyleOptionViewItem option;
    QModelIndex index;
    auto size = m_sideBarItemDelegate->sizeHint(option, index);
    EXPECT_NE(0, size.width());
    EXPECT_NE(0, size.height());
}*/

TEST_F(DFMSideBarItemDelegateTest, sizeHintForTypeTest){
    auto expectValueSeparator = QSize(200, 5) == m_sideBarItemDelegate->sizeHintForType(DFMSideBarItem::Separator);
    EXPECT_TRUE(expectValueSeparator);
    auto expectValueNoSeparator = QSize(0, 0) == m_sideBarItemDelegate->sizeHintForType(DFMSideBarItem::SidebarItem);
    EXPECT_TRUE(expectValueNoSeparator);
}

//static QString desktopPath;
//TEST_F(DFMSideBarItemDelegateTest, createEditorTest)
//{
//    //拿到桌面路径
//    desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
//    //创建一个文件夹用于测试
//    QString utDirUrl = desktopPath + "/testForUtTest";
//    QDir *utDirTest = new QDir(utDirUrl);
//    if(!utDirTest->exists())
//        EXPECT_TRUE(utDirTest->mkdir(utDirUrl));

//    Stub stub;
//    auto utUrl = static_cast<DUrl(*)()>([]{
//        DUrl tpU("/home/");
//        return tpU;
//    });

//    stub.set(ADDR(DFMSideBarItem, url), utUrl);
//    QStyleOptionViewItem option;
//    DFMSideBarView tempParent;
//    DFMSideBarModel tempModel;
//    tempParent.setModel(&tempModel);

//    DFMSideBarItem * item = new DFMSideBarItem(DUrl("/home/"), "groupName");
//    item->setData(DFMSideBarItem::Separator, DFMSideBarItem::ItemTypeRole);
//    item->setFlags(Qt::NoItemFlags);
//    tempModel.appendRow(item);

//    QWidget *widget = m_sideBarItemDelegate->createEditor(&tempParent, option, tempModel.index(0,0));
//    EXPECT_NE(nullptr, widget);


//    utDirTest->rmdir(utDirUrl);
//    delete utDirTest;
//    utDirTest = nullptr;
//}


//TEST_F(DFMSideBarItemDelegateTest, setEditorDataTest)
//{
//    QStyleOptionViewItem option;
//    QModelIndex index;
//    QWidget *widget = m_sideBarItemDelegate->createEditor(nullptr, option, index);

//    m_sideBarItemDelegate->setEditorData(widget, index);
//}

TEST_F(DFMSideBarItemDelegateTest, updateEditorGeometryTest)
{
    //拿到桌面路径
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    //创建一个文件夹用于测试
    QString utDirUrl = desktopPath + "/testForUtTest";
    QDir utDirTest(utDirUrl);
    if(!utDirTest.exists())
        EXPECT_TRUE(utDirTest.mkdir(utDirUrl));

    QStyleOptionViewItem option;
    option.rect = QRect(200, 200, 200, 100);
    Stub stub;
    auto utModel = static_cast<QAbstractItemModel*(*)()>([](){
        return dynamic_cast <QAbstractItemModel*>(&tempModel);
    });

    auto utItemUrl = static_cast<DUrl(*)()>([](){
        QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        QString utDirUrl = QString("%1/%2/testForUtTest").arg("file://").arg(desktopPath);
        return DUrl(utDirUrl);
    });
    auto utSetFixedWidth = static_cast<void(*)(int)>([](int w){
        Q_UNUSED(w);
        inThere = true;
    });
    inThere = false;
    stub.set(ADDR(QWidget, setFixedWidth), utSetFixedWidth);
    DFMSideBarView sbview;
    sbview.setItemDelegate(m_sideBarItemDelegate);
    sbview.setModel(&tempModel);
    DFMSideBarItem item(DUrl(utDirUrl), "groupName");
    item.setData(DFMSideBarItem::Separator, DFMSideBarItem::ItemTypeRole);
    item.setFlags(Qt::NoItemFlags);
    tempModel.appendRow(&item);

    QModelIndex index = tempModel.index(0, 0);
    stub.set(ADDR(DFMSideBarView, model), utModel);
    stub.set(ADDR(DFMSideBarItem, url), utItemUrl);
    QWidget *widget = m_sideBarItemDelegate->createEditor(&sbview, option, index);
    m_sideBarItemDelegate->updateEditorGeometry(widget, option, index);
    EXPECT_TRUE(inThere);

    utDirTest.remove(utDirUrl);

}

TEST_F(DFMSideBarItemDelegateTest, paint_separator)
{
    QStyleOptionViewItem option;
    option.rect = QRect(200, 200, 200, 100);
    QPainter ptr;
    Stub stub;
    auto utRestore =static_cast<void(*)()>([]{
        inThere = true;
    });
    inThere = false;
    stub.set(ADDR(QPainter, restore), utRestore);
    m_sideBarItemDelegate->paintSeparator(&ptr, option);

    EXPECT_TRUE(inThere);
}


