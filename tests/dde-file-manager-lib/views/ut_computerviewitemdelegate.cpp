/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include <QAbstractItemModel>
#include <QMouseEvent>
#include <QLineEdit>
#include "models/computermodel.h"
#include "views/computerview.h"
#include "dguiapplicationhelper.h"
#include "interfaces/dfmapplication.h"
#include "interfaces/dfilemenumanager.h"

#define private public

#include "views/computerviewitemdelegate.h"
#include "dfmevent.h"
#include "views/dfilemanagerwindow.h"
#include "stub.h"
#include "stub-ext/stubext.h"

DFM_USE_NAMESPACE
namespace  {
    class TestComputerViewItemDelegate : public testing::Test
    {
    public:
        ComputerViewItemDelegate *m_computerViewItemDelegate;
        QSharedPointer<DFileManagerWindow> m_window;
        ComputerView *m_computerView;

        virtual void SetUp() override
        {
            stub_ext::StubExt stu;
            stu.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});

            m_window = QSharedPointer<DFileManagerWindow>(new DFileManagerWindow());
            m_window->cd(DUrl(COMPUTER_ROOT));
            m_computerView = dynamic_cast<ComputerView*>(m_window->getFileView());

            m_computerViewItemDelegate = dynamic_cast<ComputerViewItemDelegate*>(m_computerView->view()->itemDelegate());
        }

        virtual void TearDown() override
        {
            m_window->clearActions();
        }
    };
}


TEST_F(TestComputerViewItemDelegate, tst_paint)
{
    Stub stub;
    static int myRole = ComputerModelItemData::Category::cat_splitter;
    static QString myFileSysType = "EXT4";
    static QString myScheme = DFMVAULT_SCHEME;
    QVariant (*ut_data)(void *obj, int role) = [](void *obj, int role) {
        switch (role) {
        case ComputerModel::DataRoles::ICategoryRole: return QVariant(myRole);
        case Qt::ItemDataRole::DisplayRole: return QVariant("utTest");
        case Qt::ItemDataRole::DecorationRole: {
            QIcon icon;
            return QVariant::fromValue(icon);
            };
        case ComputerModel::DataRoles::FileSystemRole: return QVariant(myFileSysType);
        case ComputerModel::DataRoles::SizeInUseRole: return QVariant(0);
        case ComputerModel::DataRoles::SizeTotalRole: return QVariant(1);
        case ComputerModel::DataRoles::VolumeTagRole: return QVariant("tstTagRole");
        case ComputerModel::DataRoles::SizeRole: return QVariant(true);
        case ComputerModel::DataRoles::SchemeRole: return QVariant(myScheme);
        case ComputerModel::DataRoles::ProgressRole: return QVariant(true);
        }
        return QVariant();
    };
    stub.set(ADDR(QModelIndex, data), ut_data);

//    DGuiApplicationHelper::ColorType (*ut_colorType)() = [](){return DGuiApplicationHelper::DarkType;};
//    stub.set((DGuiApplicationHelper::ColorType(DGuiApplicationHelper::*)(const QColor &))ADDR(DGuiApplicationHelper, toColorType), ut_colorType);

    QVariant (*ut_genericAttribut)() = [](){return QVariant(true);};
    stub.set(ADDR(DFMApplication, genericAttribute), ut_genericAttribut);

    QWidget widget;
    QPainter painter;
    QStyleOptionViewItem option;
    QModelIndex index;
    option.widget = &widget;
    option.state |= QStyle::StateFlag::State_Selected;
    m_computerViewItemDelegate->paint(&painter, option, index);

    myRole = ComputerModelItemData::Category::cat_widget;
    m_computerViewItemDelegate->paint(&painter, option, index);

    myRole = ComputerModelItemData::Category::cat_user_directory;
    m_computerViewItemDelegate->paint(&painter, option, index);

    option.state &= ~QStyle::StateFlag::State_Selected;
    option.state |= QStyle::StateFlag::State_MouseOver;
    m_computerViewItemDelegate->paint(&painter, option, index);

    option.state &= ~QStyle::StateFlag::State_MouseOver;
    m_computerViewItemDelegate->paint(&painter, option, index);

    myRole = ComputerModelItemData::Category::cat_external_storage;
    m_computerViewItemDelegate->paint(&painter, option, index);

    myFileSysType = "FAT32";
    m_computerViewItemDelegate->paint(&painter, option, index);

    myFileSysType = "";
    m_computerViewItemDelegate->paint(&painter, option, index);

    myScheme = "";
    m_computerViewItemDelegate->paint(&painter, option, index);
}

TEST_F(TestComputerViewItemDelegate, tst_sizeHint)
{
    Stub stub;
    static int myRole = ComputerModelItemData::Category::cat_splitter;
    QVariant (*ut_data)() = []() {
        return QVariant(myRole);
    };
    stub.set(ADDR(QModelIndex, data), ut_data);

    QStyleOptionViewItem option;
    QModelIndex index;

    QSize resultSize = m_computerViewItemDelegate->sizeHint(option, index);
    QSize expectSize = QSize(m_computerViewItemDelegate->par->width() - 12 ,45);
    EXPECT_EQ(expectSize, resultSize);

    myRole = ComputerModelItemData::Category::cat_user_directory;
    resultSize = m_computerViewItemDelegate->sizeHint(option, index);
    int sz = m_computerViewItemDelegate->par->view()->iconSize().width() * 2 + 24;
    expectSize = QSize(sz, sz);
    EXPECT_EQ(expectSize, resultSize);

    myRole = ComputerModelItemData::Category::cat_internal_storage;
    resultSize = m_computerViewItemDelegate->sizeHint(option, index);
    EXPECT_NE(expectSize, QSize(0, 0));
}

TEST_F(TestComputerViewItemDelegate, tst_createEditor)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QWidget *widget = m_computerViewItemDelegate->createEditor(nullptr, option, index);
    EXPECT_NE(nullptr, widget);
    delete widget;
}


TEST_F(TestComputerViewItemDelegate, tst_setEditorData)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QWidget *widget = m_computerViewItemDelegate->createEditor(nullptr, option, index);
    m_computerViewItemDelegate->setEditorData(widget, index);
    delete widget;
}

TEST_F(TestComputerViewItemDelegate, tst_updateEditorGeometry)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QSharedPointer<QWidget> widget = QSharedPointer<QWidget>(m_computerViewItemDelegate->createEditor(nullptr, option, index));

    Stub stub;
    static int myRole = ComputerModelItemData::Category::cat_widget;
    QVariant (*ut_data)() = []() {
        return QVariant(myRole);
    };
    stub.set(ADDR(QModelIndex, data), ut_data);

    m_computerViewItemDelegate->updateEditorGeometry(widget.data(), option, index);
    EXPECT_TRUE(widget.data()->geometry().width() > 0);

    myRole = ComputerModelItemData::Category::cat_user_directory;
    m_computerViewItemDelegate->updateEditorGeometry(widget.data(), option, index);
    EXPECT_NE(widget.data()->geometry(), option.rect);
}
