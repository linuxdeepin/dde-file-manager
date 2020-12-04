#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <QApplication>
#include <QDropEvent>
#include <QTest>
#include <stub.h>
#define private public
#define protected public

#include "models/dfmsidebarmodel.h"
#include "views/dfmsidebaritemdelegate.h"
#include "views/dfmsidebarview.h"
#include "models/trashfileinfo.h"
#include "interfaces/private/dabstractfileinfo_p.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "interfaces/dfmsidebaritem.h"

DFM_USE_NAMESPACE
using namespace testing;
namespace  {
    class TestDFMSideBarView : public Test
    {
    public:
        virtual void SetUp() override {
            testPath = QCoreApplication::applicationDirPath();
            m_view = new DFMSideBarView;
        }

        virtual void TearDown() override {
            delete m_view;
        }

        DFMSideBarView * m_view;
        QString testPath;
    };
}

TEST_F(TestDFMSideBarView, get_previous_index)
{
    ASSERT_NE(m_view, nullptr);

    QModelIndex result = m_view->getPreviousIndex();
    EXPECT_EQ(result, m_view->m_previous);
}

TEST_F(TestDFMSideBarView, get_current_index)
{
    ASSERT_NE(m_view, nullptr);

    QModelIndex result = m_view->getCurrentIndex();
    EXPECT_EQ(result, m_view->m_current);
}

TEST_F(TestDFMSideBarView, set_current_changed)
{
    ASSERT_NE(m_view, nullptr);

    QModelIndex newMode;
    m_view->currentChanged(newMode);

    EXPECT_EQ(newMode, m_view->m_previous);
}

TEST_F(TestDFMSideBarView, get_item_at)
{
    ASSERT_NE(m_view, nullptr);

    QPoint pt(1,1);
    DFMSideBarItem *result = m_view->itemAt(pt);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestDFMSideBarView, get_index_at)
{
    ASSERT_NE(m_view, nullptr);

    m_view->indexAt(QPoint(1,1));
}

TEST_F(TestDFMSideBarView, get_drop_data)
{
    ASSERT_NE(m_view, nullptr);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();
    DFMSideBarView *t_p = bar->m_sidebarView;

    DUrlList srcUrls;
    srcUrls << DUrl("test1")<<DUrl("test2");
    DUrl dstUrl("/home/test/");
    TrashFileInfo *info = new TrashFileInfo(dstUrl);
    Q_UNUSED(info)

    bool resutl = t_p->onDropData(srcUrls, dstUrl, Qt::IgnoreAction);
    EXPECT_FALSE(resutl);
}

TEST_F(TestDFMSideBarView, drop_mime_data)
{
    ASSERT_NE(m_view, nullptr);

    m_view->m_urlsForDragEvent.clear();
    Qt::DropAction result = m_view->canDropMimeData(nullptr, nullptr, Qt::IgnoreAction);
    EXPECT_EQ(result, Qt::IgnoreAction);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();
    DFMSideBarView *t_p = bar->m_sidebarView;
    DFMSideBarItem *item = DFMSideBarItem::createSeparatorItem(QString("Trash"));
    ASSERT_NE(item, nullptr);

    DUrlList srcUrls;
    srcUrls << DUrl("test1")<<DUrl("test2");
    DUrl dstUrl("/home/test/");
    TrashFileInfo *info = new TrashFileInfo(dstUrl);
    Q_UNUSED(info)

    QMimeData data;
    t_p->m_urlsForDragEvent << QUrl("/home/test/aaa") << QUrl("/home/test/bbb");

    result = t_p->canDropMimeData(item, &data, Qt::IgnoreAction);
    EXPECT_EQ(result, Qt::IgnoreAction);
}

TEST_F(TestDFMSideBarView, get_acceptte_drag)
{
    ASSERT_NE(m_view, nullptr);

    QPointF pos(1,1);
    Qt::DropActions actions = Qt::MoveAction;
    QMimeData data;
    Qt::MouseButtons buttons = Qt::LeftButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QDropEvent event(pos, actions, &data, buttons, modifiers);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();
    DFMSideBarView *t_p = bar->m_sidebarView;
    ASSERT_NE(t_p, nullptr);

    bool result = t_p->isAccepteDragEvent(&event);
    EXPECT_FALSE(result);
}

TEST_F(TestDFMSideBarView, get_row_changed)
{
    ASSERT_NE(m_view, nullptr);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();
    DFMSideBarView *t_p = bar->m_sidebarView;
    ASSERT_NE(t_p, nullptr);

    if(t_p->m_strItemUniqueKey.isEmpty()) {
        t_p->m_strItemUniqueKey = QString("testKey");
    }
    t_p->onRowCountChanged();
}

TEST_F(TestDFMSideBarView, featch_drag_memory)
{
    ASSERT_NE(m_view, nullptr);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();
    DFMSideBarView *t_p = bar->m_sidebarView;
    ASSERT_NE(t_p, nullptr);

    EXPECT_NO_FATAL_FAILURE(t_p->fetchDragEventUrlsFromSharedMemory());
}

TEST_F(TestDFMSideBarView, check_op_time)
{
    ASSERT_NE(m_view, nullptr);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();
    DFMSideBarView *t_p = bar->m_sidebarView;
    ASSERT_NE(t_p, nullptr);

    QThread::msleep(300);
    bool result = t_p->checkOpTime();
    EXPECT_TRUE(result);
    result = t_p->checkOpTime();
    EXPECT_FALSE(result);
}
