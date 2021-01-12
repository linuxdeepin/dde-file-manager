#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QAction>

#include "views/dfmactionbutton.h"
namespace  {
    class DFMActionButtonTest : public testing::Test
    {
    public:        
        virtual void SetUp() override
        {
            m_button = new DFMActionButton;
        }

        virtual void TearDown() override
        {
            delete m_button;
        }
        DFMActionButton *m_button;
    };
}

TEST_F(DFMActionButtonTest,set_current_action)
{
    ASSERT_NE(nullptr,m_button);

    QAction action("test");
    m_button->setAction(&action);

    QAction *result = m_button->action();
    EXPECT_EQ(result, &action);

}

TEST_F(DFMActionButtonTest,get_current_action)
{
    ASSERT_NE(nullptr,m_button);

    QAction action("copy");
    m_button->setAction(&action);

    QAction *result = m_button->action();
    EXPECT_EQ(result, &action);
}



