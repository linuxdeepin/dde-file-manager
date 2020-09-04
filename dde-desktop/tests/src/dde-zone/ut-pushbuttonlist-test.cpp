#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QDBusPendingReply>

#define private public

#include "../dde-zone/pushbuttonlist.h"

using namespace testing;

namespace  {
     class PushButtonListTest : public Test {
     public:
         PushButtonListTest() : Test() {}

         virtual void SetUp() override
         {
             m_widget = new HoverWidget;
             m_lable = new QLabel;
             m_button = new PushButtonList(new HoverWidget, false, false, m_lable);
             m_button1 = new PushButtonList(new HoverWidget, true, false, m_lable);
             m_button2 = new PushButtonList(new HoverWidget, false, true, m_lable);
             m_button3 = new PushButtonList(new HoverWidget, true, true, m_lable);
         }

         virtual void TearDown() override
         {
             delete m_widget;
             delete m_lable;
             delete m_button;
             delete m_button1;
             delete m_button2;
             delete m_button3;
         }

         HoverWidget *m_widget = nullptr;
         QLabel *m_lable = nullptr;
         PushButtonList *m_button = nullptr;
         PushButtonList *m_button1 = nullptr;
         PushButtonList *m_button2 = nullptr;
         PushButtonList *m_button3 = nullptr;
     };
}

TEST_F(PushButtonListTest, set_buttonclickstatus)
{
    m_button->setButtonClickStatus(true);
    EXPECT_TRUE(m_button->buttonHasClicked);
}

TEST_F(PushButtonListTest, button_clickstatus)
{
    m_button->setButtonClickStatus(true);
    m_button->buttonClickStatus();
    EXPECT_TRUE(m_button->buttonHasClicked);
}

TEST_F(PushButtonListTest, onbutton_clicked)
{
    m_button->addButton("button", "TopLeft", false);
    m_button->onButtonClicked("TopLeft", 0);
    QDBusPendingReply<QString> dpret = m_button->m_zoneInterface->TopLeftAction();
    EXPECT_EQ(m_button->corner, PushButtonList::TopLeft);
    EXPECT_EQ(dpret.value().toStdString(), "TopLeft");
    EXPECT_EQ(m_button->m_SelectedText->text(), "button");
    EXPECT_TRUE(m_button->buttonHasClicked);
    EXPECT_FALSE(m_button->m_ParentItem->isVisible());

    m_button1->addButton("button", "TopRight", false);
    m_button1->onButtonClicked("TopRight", 0);
    QDBusPendingReply<QString> dpret1 = m_button1->m_zoneInterface->TopRightAction();
    EXPECT_EQ(m_button1->corner, PushButtonList::TopRight);
    EXPECT_EQ(dpret1.value().toStdString(), "TopRight");
    EXPECT_EQ(m_button1->m_SelectedText->text(), "button");
    EXPECT_TRUE(m_button1->buttonHasClicked);
    EXPECT_FALSE(m_button1->m_ParentItem->isVisible());

    m_button2->addButton("button", "BottomLeft", false);
    m_button2->onButtonClicked("BottomLeft", 0);
    QDBusPendingReply<QString> dpret2 = m_button2->m_zoneInterface->BottomLeftAction();
    EXPECT_EQ(m_button2->corner, PushButtonList::BottomLeft);
    EXPECT_EQ(dpret2.value().toStdString(), "BottomLeft");
    EXPECT_EQ(m_button2->m_SelectedText->text(), "button");
    EXPECT_TRUE(m_button2->buttonHasClicked);
    EXPECT_FALSE(m_button2->m_ParentItem->isVisible());

    m_button3->addButton("button", "BottomRight", false);
    m_button3->onButtonClicked("BottomRight", 0);
    QDBusPendingReply<QString> dpret3 = m_button3->m_zoneInterface->BottomRightAction();
    EXPECT_EQ(m_button3->corner, PushButtonList::BottomRight);
    EXPECT_EQ(dpret3.value().toStdString(), "BottomRight");
    EXPECT_EQ(m_button3->m_SelectedText->text(), "button");
    EXPECT_TRUE(m_button3->buttonHasClicked);
    EXPECT_FALSE(m_button3->m_ParentItem->isVisible());
}
