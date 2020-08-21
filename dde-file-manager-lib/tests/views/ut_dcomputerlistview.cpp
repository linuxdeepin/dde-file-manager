#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "views/dcompleterlistview.h"


DFM_USE_NAMESPACE
namespace  {
    class TestDCompleterListView : public testing::Test
    {
    public:
        QSharedPointer<DCompleterListView> m_completer;

        virtual void SetUp() override
        {
            m_completer = QSharedPointer<DCompleterListView>(new DCompleterListView());

            std::cout << "start TestDCompleterListView" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDCompleterListView" << std::endl;
        }
    };
}


TEST_F(TestDCompleterListView, can_show_hide)
{
    EXPECT_NO_FATAL_FAILURE(m_completer->showMe());
    EXPECT_NO_FATAL_FAILURE(m_completer->hideMe());
}



