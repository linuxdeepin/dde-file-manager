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

#include <QMouseEvent>
#include <QTest>

#define private public

#include "views/computerview.h"
#include "models/computermodel.h"
#include "dfmevent.h"
#include "dfilemenu.h"
#include "views/dfilemanagerwindow.h"
#include "views/windowmanager.h"
#include "views/dfmopticalmediawidget.h"
#include "stub.h"

DFM_USE_NAMESPACE
namespace  {
    class ComputerViewTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            m_window = QSharedPointer<DFileManagerWindow>(new DFileManagerWindow());
            m_window->cd(DUrl(COMPUTER_ROOT));
            m_computerView = dynamic_cast<ComputerView*>(m_window->getFileView());
        }

        virtual void TearDown() override
        {
        }
        ComputerView *m_computerView;
        QSharedPointer<DFileManagerWindow> m_window;
    };
}


TEST_F(ComputerViewTest, get_widget)
{
    ASSERT_NE(nullptr, m_computerView);
    ASSERT_NE(nullptr, m_computerView->widget());
}

TEST_F(ComputerViewTest, get_root_url)
{
    DUrl url(COMPUTER_ROOT);
    DUrl result = m_computerView->rootUrl();
    EXPECT_EQ(url, result);
}

TEST_F(ComputerViewTest, set_root_url)
{
    DUrl url("test");
    bool result = m_computerView->setRootUrl(url);
    EXPECT_FALSE(result);
}

TEST_F(ComputerViewTest, get_view_list)
{
    EXPECT_NE(nullptr, m_computerView->view());
}

TEST_F(ComputerViewTest, call_context_menu)
{
    EXPECT_NO_FATAL_FAILURE(m_computerView->contextMenu(QPoint(0, 0)));

    Stub stub;
    static bool myValid = true;
    bool (*ut_isValid)() = [](){return myValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    bool (*ut_tabAddableByWinId)() = [](){return false;};
    stub.set(ADDR(WindowManager, tabAddableByWinId), ut_tabAddableByWinId);

    static QString mySchemeRole = DFMVAULT_SCHEME;
    QVariant (*ut_data)(void *, uint64_t) = [](void *obj, uint64_t arole) {
        switch (arole) {
        case ComputerModel::DataRoles::ActionVectorRole: {
            QVector<MenuAction> menus;
            MenuAction menu;
            menus.append(menu);
            return QVariant::fromValue(menus);
            };
        case ComputerModel::DataRoles::VolumeTagRole: return QVariant("srTextFile");
        case ComputerModel::DataRoles::DiscUUIDRole: return QVariant("");
        case ComputerModel::DataRoles::DiscOpticalRole: return QVariant(false);
        case ComputerModel::DataRoles::SizeTotalRole: return QVariant(0);
        case ComputerModel::DataRoles::SchemeRole: return QVariant(mySchemeRole);
        }
        return QVariant();
    };
    stub.set(ADDR(QModelIndex, data), ut_data);

    static bool myCallExec = false;
    void (*ut_exec)() = [](){myCallExec = true;};
    stub.set((QAction*(DFileMenu::*)(const QPoint&, QAction*))ADDR(DFileMenu, exec), ut_exec);

    bool t_isContains = false;
    bool t_tstValue = false;
    if (DFMOpticalMediaWidget::g_mapCdStatusInfo.contains(QString("srTextFile"))) {
        t_isContains = true;
        t_tstValue = DFMOpticalMediaWidget::g_mapCdStatusInfo[QString("srTextFile")].bBurningOrErasing;
    }
    DFMOpticalMediaWidget::g_mapCdStatusInfo[QString("srTextFile")].bBurningOrErasing = true;

    EXPECT_NO_FATAL_FAILURE(m_computerView->contextMenu(QPoint(100, 100)));

    bool (*ut_isEmpty)() = [](){return false;};
    stub.set(ADDR(QList<QAction*>, isEmpty), ut_isEmpty);

    myCallExec = false;
    mySchemeRole = QString("other");
    EXPECT_NO_FATAL_FAILURE(m_computerView->contextMenu(QPoint(100, 100)));
    EXPECT_TRUE(myCallExec);

    if (t_isContains) {
        DFMOpticalMediaWidget::g_mapCdStatusInfo[QString("srTextFile")].bBurningOrErasing = t_tstValue;
    } else {
        DFMOpticalMediaWidget::g_mapCdStatusInfo.remove(QString("srTextFile"));
    }
}

TEST_F(ComputerViewTest, call_rename_requested)
{
    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  nullptr, m_computerView->rootUrl()).data()));
    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  m_computerView, m_computerView->rootUrl()).data()));

    Stub stub;
    static bool myValid = true;
    bool (*ut_isValid)() = [](){return myValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  m_computerView, DUrl("C://"))));
}

TEST_F(ComputerViewTest, call_resize_event)
{
    EXPECT_NO_FATAL_FAILURE(m_computerView->resize(100, 100));
    EXPECT_NO_FATAL_FAILURE(m_computerView->resize(10, 10));
    QSize size = m_computerView->size();
    EXPECT_EQ(10, size.width());
    EXPECT_EQ(10, size.height());
}

TEST_F(ComputerViewTest, call_event_filter)
{
    ASSERT_NE(nullptr, m_computerView);

    QTest::keyPress(m_computerView->view()->viewport(), Qt::Key_Escape, Qt::AltModifier);
    QTest::mouseRelease(m_computerView->view()->viewport(), Qt::LeftButton);
    EXPECT_TRUE(m_computerView->view()->selectionModel()->selectedIndexes().isEmpty());

    emit m_computerView->m_model->itemCountChanged(2);

    QItemSelection selected,deselected;
    emit m_computerView->view()->selectionModel()->selectionChanged(selected, deselected);
}

TEST_F(ComputerViewTest, mode_view_single_simple)
{
    ASSERT_NE(nullptr, m_computerView);

    emit m_computerView->m_model->itemCountChanged(0);

    Stub stub;
    static bool myHasSelection = true;
    bool (*ut_hasSelection)() = [](){return myHasSelection;};
    stub.set(ADDR(QItemSelectionModel, hasSelection), ut_hasSelection);

    QItemSelection selected, deselected;
    emit m_computerView->m_view->selectionModel()->selectionChanged(selected, deselected);
}

//TEST_F(ComputerViewTest, mode_view_single_click)
//{
//    ASSERT_NE(nullptr, m_computerView);

//    QModelIndex index;
//    emit m_computerView->m_view->clicked(index);

//    Stub stub;
//    static bool myValid = true;
//    bool (*ut_isValid)() = [](){return myValid;};
//    stub.set(ADDR(DUrl, isValid), ut_isValid);

//    static bool myCheckGvfsMountfileBusy = false;
//    bool (*ut_checkGvfsMountfileBusy)() = [](){return myCheckGvfsMountfileBusy;};
//    stub.set((bool(DFileService::*)(const DUrl &, const bool))ADDR(DFileService, checkGvfsMountfileBusy), ut_checkGvfsMountfileBusy);

//    static QString myPath = QString("file:/userdir");
//    QString (*ut_path)() = [](){return myPath;};
//    stub.set(ADDR(QUrl, path), ut_path);

//    static bool myActinOpen = false;
//    void (*ut_actionOpen)() = [](){myActinOpen = true;};
//    stub.set(ADDR(AppController, actionOpen), ut_actionOpen);

//    static bool myActionOpenDisk = false;
//    void (*ut_actionOpenDisk)() = [](){myActionOpenDisk = true;};
//    stub.set(ADDR(AppController, actionOpenDisk), ut_actionOpenDisk);

//    emit m_computerView->m_view->doubleClicked(index);

//    myPath = QString("file:/home");
//    static QString myScheme = DFMVAULT_SCHEME;
//    QString (*ut_scheme)() = [](){return myScheme;};
//    stub.set(ADDR(QUrl, scheme), ut_scheme);
//    emit m_computerView->m_view->doubleClicked(index);

//    myScheme = "file";
//    emit m_computerView->m_view->doubleClicked(index);

//    EXPECT_TRUE(myActinOpen);
//    EXPECT_TRUE(myActionOpenDisk);

//    myCheckGvfsMountfileBusy = true;
//    emit m_computerView->m_view->doubleClicked(index);

//    myValid = false;
//    emit m_computerView->m_view->doubleClicked(index);
//}

TEST_F(ComputerViewTest, mode_view_single_triggered)
{
    ASSERT_NE(nullptr, m_computerView);

    Stub stub;
    static bool myHasSelection = true;
    bool (*ut_hasSelection)() = [](){return myHasSelection;};
    stub.set(ADDR(QItemSelectionModel, hasSelection), ut_hasSelection);

    static QString myPath = QString("file:/userdir");
    QString (*ut_path)() = [](){return myPath;};
    stub.set(ADDR(QUrl, path), ut_path);

    static bool myActionProperty = false;
    void (*ut_actionProperty)() = [](){myActionProperty = true;};
    stub.set(ADDR(AppController, actionProperty), ut_actionProperty);

    static bool myActionOpenInNewTab = false;
    void (*ut_actionOpenInNewTab)() = [](){myActionOpenInNewTab = true;};
    stub.set(ADDR(AppController, actionOpenInNewTab), ut_actionOpenInNewTab);

    static bool myActionOpenDiskInNewTab = false;
    void (*ut_actionOpenDiskInNewTab)() = [](){myActionOpenDiskInNewTab = true;};
    stub.set(ADDR(AppController, actionOpenDiskInNewTab), ut_actionOpenDiskInNewTab);

    static bool myActionOpenInNewWindow = false;
    void (*ut_actionOpenInNewWindow)() = [](){myActionOpenInNewWindow = true;};
    stub.set(ADDR(AppController, actionOpenInNewWindow), ut_actionOpenInNewWindow);

    static bool myActionOpenDiskInNewWindow = false;
    void (*ut_actionOpenDiskInNewWindow)() = [](){myActionOpenDiskInNewWindow = true;};
    stub.set(ADDR(AppController, actionOpenDiskInNewWindow), ut_actionOpenDiskInNewWindow);

    QList<QAction*> actions = m_computerView->m_view->actions();
    for (auto action : actions) {
        if (action)
            emit action->triggered();
    }

    myPath = QString("file:/home");
    for (auto action : actions) {
        if (action)
            emit action->triggered();
    }

    myHasSelection = false;
    for (auto action : actions) {
        if (action)
            emit action->triggered();
    }
    EXPECT_TRUE(myActionProperty);
    EXPECT_TRUE(myActionOpenInNewTab);
    EXPECT_TRUE(myActionOpenDiskInNewTab);
    EXPECT_TRUE(myActionOpenInNewWindow);
    EXPECT_TRUE(myActionOpenDiskInNewWindow);
}
