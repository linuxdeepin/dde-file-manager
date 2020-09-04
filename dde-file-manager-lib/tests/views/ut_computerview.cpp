#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "views/computerview.h"
#include "dfmevent.h"
#include <QMouseEvent>
#include "views/dfilemanagerwindow.h"



DFM_USE_NAMESPACE
namespace  {
    class TestComputerView : public testing::Test
    {
    public:
        ComputerView *m_computerView;
        QSharedPointer<DFileManagerWindow> m_window;

        virtual void SetUp() override
        {
            m_window = QSharedPointer<DFileManagerWindow>(new DFileManagerWindow());
            m_window->cd(DUrl(COMPUTER_ROOT));
            m_computerView = dynamic_cast<ComputerView*>(m_window->getFileView());

            std::cout << "start TestComputerView" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestComputerView" << std::endl;
        }
    };
}


TEST_F(TestComputerView, getWidget)
{
    ASSERT_NE(nullptr, m_computerView->widget());
}

TEST_F(TestComputerView, tstRootUrl)
{
    //! only computer path can be set.
    DUrl url(COMPUTER_ROOT);
    m_computerView->setRootUrl(url);
    EXPECT_EQ(url, m_computerView->rootUrl());
}

TEST_F(TestComputerView, getViewList)
{
    EXPECT_NE(nullptr, m_computerView->view());
}

TEST_F(TestComputerView, tstContextMenu)
{
    EXPECT_NO_FATAL_FAILURE(m_computerView->contextMenu(QPoint(0, 0)));
    EXPECT_NO_FATAL_FAILURE(m_computerView->contextMenu(QPoint(100, 100)));
}

TEST_F(TestComputerView, tstRenameRequest)
{
    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  nullptr, m_computerView->rootUrl()).data()));
    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  m_computerView, m_computerView->rootUrl()).data()));
    EXPECT_NO_FATAL_FAILURE(m_computerView->onRenameRequested(*dMakeEventPointer<DFMUrlBaseEvent>(
                                                                  m_computerView, DUrl("C://"))));
}

TEST_F(TestComputerView, tstResizeEvent)
{
    EXPECT_NO_FATAL_FAILURE(m_computerView->resize(10, 10));
    QSize size = m_computerView->size();
    EXPECT_EQ(10, size.width());
    EXPECT_EQ(10, size.height());
}

