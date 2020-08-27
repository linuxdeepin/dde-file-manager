#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public

#include "../../../dde-zone/hotzone.h"


using namespace testing;


namespace  {
     class PushButtonListTest : public Test{
     public:
         PushButtonListTest():Test(){

         }
         virtual void SetUp() override{
             mwidget = new HoverWidget;
             mlable = new QLabel;
             plist = new PushButtonList(new HoverWidget,false,false,mlable);
         }
         virtual void TearDown() override{
             delete mwidget;
             delete mlable;
             delete plist;
         }

         HoverWidget* mwidget;
         QLabel* mlable;
         PushButtonList* plist;
     };
}


TEST_F(PushButtonListTest,setButtonClickStatus)
{
    plist->setButtonClickStatus(true);
    EXPECT_TRUE(plist->buttonHasClicked);
}


TEST_F(PushButtonListTest,buttonClickStatus)
{
    plist->setButtonClickStatus(true);
    plist->buttonClickStatus();
    EXPECT_TRUE(plist->buttonHasClicked);
}
