#include <gtest/gtest.h>
#include <qabstractitemmodel.h>
#include <QScrollBar>

#include "stub.h"
#include "views/dfilemanagerwindow.h"
#include "dfilestatisticsjob.h"
#include "interfaces/dfilemenu.h"
#include "views/dfmaddressbar.h"

#define protected public
#include "dfmcrumbbar.h"
#include "private/dfmcrumbbar_p.h"

DFM_USE_NAMESPACE

namespace  {
class TestDFMCrumbBarPrivate : public testing::Test
{
    void SetUp() override
    {
        m_pBar = new DFMCrumbBar();
        m_pTester = new DFMCrumbBarPrivate(m_pBar);
        std::cout << "start TestDFMCrumbBarPrivate";
    }
    void TearDown() override
    {
        if (m_pBar)
            delete m_pTester;
        m_pBar = nullptr;
        m_pTester = nullptr;
        std::cout << "end TestDFMCrumbBarPrivate";
    }
public:
    DFMCrumbBar         *m_pBar;
    DFMCrumbBarPrivate *m_pTester;
};
}

TEST_F(TestDFMCrumbBarPrivate, testSetClickableAreaEnabled)
{
    QStyleOptionViewItem item;
    QModelIndex index;
    void(*stu_paint)(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) = [](QPainter *, const QStyleOptionViewItem &, const QModelIndex &) {
        int a = 0;
    };
    Stub stu;
    typedef  void(*pfun)(QPainter *, const QStyleOptionViewItem &, const QModelIndex &);
    stu.set((pfun)ADDR(DStyledItemDelegate, paint), stu_paint);
    m_pTester->crumbListView.itemDelegateForRow(0)->paint(nullptr, item, index);

    emit m_pTester->crumbListView.clicked(QModelIndex());
    emit m_pTester->leftArrow.click();
    emit m_pTester->rightArrow.click();
    emit m_pTester->crumbListView.horizontalScrollBar()->valueChanged(0);
    emit m_pTester->addressBar->returnPressed();
    emit m_pTester->addressBar->escKeyPressed();
    emit m_pTester->addressBar->lostFocus();
    emit m_pTester->addressBar->clearButtonPressed();

    EXPECT_NO_FATAL_FAILURE(m_pTester->setClickableAreaEnabled(true));
}

namespace  {
class TestDFMCrumbBar : public testing::Test
{
public:
    void SetUp() override
    {
        m_pWidget = new QWidget();
        m_pTester = new DFMCrumbBar(m_pWidget);
        std::cout << "start TestDFMCrumbBar";
    }
    void TearDown() override
    {
        if (m_pTester) {
            delete m_pTester;
            m_pTester = nullptr;
        }
        if (m_pWidget) {
            delete m_pWidget;
            m_pWidget = nullptr;
        }
        std::cout << "end TestDFMCrumbBar";
    }
public:
    QWidget *m_pWidget;
    DFMCrumbBar *m_pTester;
};
}

TEST_F(TestDFMCrumbBar, testShowAddressBar)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->showAddressBar("utest"));
}

TEST_F(TestDFMCrumbBar, testShowAddressBar2)
{
    DUrl url("file:///test");
    EXPECT_NO_FATAL_FAILURE(m_pTester->showAddressBar(url));
}

TEST_F(TestDFMCrumbBar, testHideAddressBar)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->hideAddressBar());
}

TEST_F(TestDFMCrumbBar, testUpdateCrumbs)
{
    DUrl url("file:///test");
    EXPECT_NO_FATAL_FAILURE(m_pTester->updateCrumbs(url));
}

TEST_F(TestDFMCrumbBar, testPlayAddressBarAnimation)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->playAddressBarAnimation());
}

TEST_F(TestDFMCrumbBar, testStopAddressBarAnimation)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->stopAddressBarAnimation());
}

TEST_F(TestDFMCrumbBar, testUpdateCurrentUrl)
{
    DUrl url("file:///test");
    EXPECT_NO_FATAL_FAILURE(m_pTester->updateCurrentUrl(url));
}

TEST_F(TestDFMCrumbBar, testMousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->mousePressEvent(&event));
}

TEST_F(TestDFMCrumbBar, testMouseReleaseEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->mouseReleaseEvent(&event));
}

TEST_F(TestDFMCrumbBar, testResizeEvent)
{
    QResizeEvent event(QSize(100, 100), QSize(10, 10));
    EXPECT_NO_FATAL_FAILURE(m_pTester->resizeEvent(&event));
}

TEST_F(TestDFMCrumbBar, testShowEvent)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(m_pTester->showEvent(&event));
}

TEST_F(TestDFMCrumbBar, testEventFilter)
{
    QEvent event(QEvent::MouseButtonPress);
    EXPECT_NO_FATAL_FAILURE(m_pTester->eventFilter(nullptr, &event));
}

TEST_F(TestDFMCrumbBar, testOnListViewContextMenu)
{
    QPoint point(0, 0);

    bool(*stu_isValid)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(QModelIndex, isValid), stu_isValid);

    bool(*stu_toBool)() = []()->bool{
        return true;
    };
    Stub stu2;
    stu2.set(ADDR(QVariant, toBool), stu_toBool);

    quint64(*stu_windowId)() = []()->quint64 {
        return 0;
    };
    Stub stu4;
    stu4.set(ADDR(DFileManagerWindow, windowId), stu_windowId);

    QAction*(*stu_exec)(const QPoint &, QAction *) = [](const QPoint &, QAction *ac = nullptr)->QAction* {
        Q_UNUSED(ac);
        return nullptr;
    };
    Stub stu5;
    stu5.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), stu_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->onListViewContextMenu(point));
}
