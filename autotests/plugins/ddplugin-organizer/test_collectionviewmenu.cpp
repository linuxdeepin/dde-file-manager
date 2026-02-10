// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectionviewmenu.h"
#include "view/collectionview.h"
#include "view/collectionwidget.h"
#include "mode/collectiondataprovider.h"
#include "interface/canvasinterface.h"
#include "models/collectionmodel.h"

#include <QModelIndex>
#include <QPoint>
#include <QTest>
#include <QWidget>
#include <QTimer>

#include "stubext.h"
#include "gtest/gtest.h"

using namespace ddplugin_organizer;

// Mock classes for testing

class MockCollectionDataProvider : public CollectionDataProvider
{
public:
    MockCollectionDataProvider(QObject *parent = nullptr) : CollectionDataProvider(parent) {}
    
    // 实现纯虚函数
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) override { Q_UNUSED(oldUrl); Q_UNUSED(newUrl); return QString(); }
    QString append(const QUrl &url) override { Q_UNUSED(url); return QString(); }
    QString prepend(const QUrl &url) override { Q_UNUSED(url); return QString(); }
    void insert(const QUrl &url, const QString &key, const int index) override { Q_UNUSED(url); Q_UNUSED(key); Q_UNUSED(index); }
    QString remove(const QUrl &url) override { Q_UNUSED(url); return QString(); }
    QString change(const QUrl &url) override { Q_UNUSED(url); return QString(); }
};

class UT_CollectionViewMenu : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new MockCollectionDataProvider();
        widget = new CollectionWidget("test-uuid", provider);
        view = widget->view();

        // Create and set a mock model for the view to prevent null model crashes
        CollectionModel *mockModel = new CollectionModel();
        stub.set_lamda(&CollectionView::model, [mockModel]() {
            __DBG_STUB_INVOKE__
            return mockModel;
        });

        // mock the select model to prevent null selection model crashes
        QItemSelectionModel *mockSelectionModel = new QItemSelectionModel(mockModel);
        stub.set_lamda(&CollectionView::selectionModel, [mockSelectionModel]() {
            __DBG_STUB_INVOKE__
            return mockSelectionModel;
        });

        menu = new CollectionViewMenu(view);
        
        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }
    
    void TearDown() override
    {
        delete menu;
        delete widget;
        delete provider;

        stub.clear();
    }
    
    CollectionViewMenu* menu;
    CollectionView* view;
    CollectionWidget* widget;
    MockCollectionDataProvider* provider;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionViewMenu, ConstructorTest)
{
    EXPECT_NE(menu, nullptr);
    EXPECT_EQ(menu->getCanvasView(), nullptr);
}

TEST_F(UT_CollectionViewMenu, DisableMenuTest)
{
    bool result = CollectionViewMenu::disableMenu();
    
    for (int i = 0; i < 5; ++i) {
        bool result2 = CollectionViewMenu::disableMenu();
        (void)result2;
    }
}

TEST_F(UT_CollectionViewMenu, EmptyAreaMenuTest)
{
    menu->emptyAreaMenu();
    QTest::qWait(50);
    
    for (int i = 0; i < 3; ++i) {
        menu->emptyAreaMenu();
        QTest::qWait(20);
    }
    
    bool disableResult = CollectionViewMenu::disableMenu();
    menu->emptyAreaMenu();
    QTest::qWait(20);
}

TEST_F(UT_CollectionViewMenu, NormalMenuTest)
{
    QModelIndex modelIndex;
    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    QPoint gridPos(10, 10);
    
    menu->normalMenu(modelIndex, itemFlags, gridPos);
    QTest::qWait(50);
    
    menu->normalMenu(QModelIndex(), Qt::NoItemFlags, QPoint(0, 0));
    QTest::qWait(50);
    
    for (int i = 0; i < 3; ++i) {
        menu->normalMenu(modelIndex, static_cast<Qt::ItemFlags>(Qt::ItemIsEnabled | Qt::ItemIsSelectable | i), gridPos + QPoint(i * 5, i * 5));
        QTest::qWait(20);
    }
}

TEST_F(UT_CollectionViewMenu, GetCanvasViewTest)
{
    QWidget* canvasView = menu->getCanvasView();
    EXPECT_EQ(canvasView, nullptr);
    
    QWidget* canvasView2 = menu->getCanvasView();
    EXPECT_EQ(canvasView2, nullptr);
    
    for (int i = 0; i < 5; ++i) {
        QWidget* canvasView = menu->getCanvasView();
        (void)canvasView;
    }
}

TEST_F(UT_CollectionViewMenu, MenuWithDifferentFlagsTest)
{
    QModelIndex modelIndex;
    QPoint gridPos(5, 5);
    
    Qt::ItemFlags flags[] = {
        Qt::NoItemFlags,
        Qt::ItemIsSelectable,
        Qt::ItemIsEditable,
        Qt::ItemIsDragEnabled,
        Qt::ItemIsDropEnabled,
        Qt::ItemIsUserCheckable,
        Qt::ItemIsEnabled | Qt::ItemIsSelectable,
        Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
    };
    
    for (Qt::ItemFlags flag : flags) {
        menu->normalMenu(modelIndex, flag, gridPos);
        QTest::qWait(10);
    }
}

TEST_F(UT_CollectionViewMenu, MenuWithDifferentPositionsTest)
{
    QModelIndex modelIndex;
    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    QPoint positions[] = {
        QPoint(0, 0),
        QPoint(10, 10),
        QPoint(-5, -5),
        QPoint(100, 100),
        QPoint(999, 999)
    };
    
    for (const QPoint& pos : positions) {
        menu->normalMenu(modelIndex, itemFlags, pos);
        QTest::qWait(10);
    }
}

TEST_F(UT_CollectionViewMenu, MultipleMenusTest)
{
    CollectionViewMenu* menu1 = new CollectionViewMenu(view);
    CollectionViewMenu* menu2 = new CollectionViewMenu(view);
    CollectionViewMenu* menu3 = new CollectionViewMenu(view);
    
    EXPECT_NE(menu1, nullptr);
    EXPECT_NE(menu2, nullptr);
    EXPECT_NE(menu3, nullptr);
    
    menu1->emptyAreaMenu();
    menu2->emptyAreaMenu();
    menu3->emptyAreaMenu();
    QTest::qWait(50);
    
    delete menu1;
    delete menu2;
    delete menu3;
}

TEST_F(UT_CollectionViewMenu, StressTest)
{
    for (int i = 0; i < 10; ++i) {
        menu->emptyAreaMenu();
        QTest::qWait(5);
        
        QModelIndex modelIndex;
        menu->normalMenu(modelIndex, Qt::ItemIsEnabled, QPoint(i, i));
        QTest::qWait(5);
    }
}

TEST_F(UT_CollectionViewMenu, EdgeCasesTest)
{
    QModelIndex invalidIndex;
    Qt::ItemFlags emptyFlags;
    QPoint invalidPos(-1000, -1000);
    
    menu->normalMenu(invalidIndex, emptyFlags, invalidPos);
    QTest::qWait(10);
    
    menu->normalMenu(QModelIndex(), static_cast<Qt::ItemFlags>(0xFFFFFFFF), QPoint(0, 0));
    QTest::qWait(10);
    
    bool disableResult = CollectionViewMenu::disableMenu();
    menu->emptyAreaMenu();
    QTest::qWait(10);
}
