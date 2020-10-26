#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QMouseEvent>
#include <QTest>

#define private public

#include "views/computerview.h"
#include "models/computermodel.h"
#include "dfmevent.h"
#include "views/dfilemanagerwindow.h"

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
    EXPECT_NO_FATAL_FAILURE(m_computerView->contextMenu(QPoint(100, 100)));
}

TEST_F(ComputerViewTest, call_rename_requested)
{
    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  nullptr, m_computerView->rootUrl()).data()));
    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  m_computerView, m_computerView->rootUrl()).data()));
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
