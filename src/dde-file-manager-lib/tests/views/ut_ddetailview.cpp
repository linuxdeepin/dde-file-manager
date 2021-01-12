#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "views/ddetailview.h"


namespace  {
    class TestDDetailView : public testing::Test
    {
    public:
        QSharedPointer<DDetailView> m_detailView;

        virtual void SetUp() override
        {
            m_detailView = QSharedPointer<DDetailView>(new DDetailView());

            std::cout << "start TestDDetailView" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDDetailView" << std::endl;
        }
    };
}

TEST_F(TestDDetailView, can_create_obj)
{
    ASSERT_NE(nullptr, m_detailView);
}

