#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "../../dde-wallpaper-chooser/checkbox.h"

using namespace testing;

namespace  {
      class CheckBoxTest : public Test{
      public:
          CheckBoxTest() : Test(){

          }

          virtual void SetUp() override{

              cbox = new CheckBox;
              cbox1 = new CheckBox("hhh");
          }

          virtual void TearDown() override{
              delete cbox;
              delete cbox1;
          }

          CheckBox* cbox;
          CheckBox* cbox1;
      };
}
TEST_F(CheckBoxTest,checkbox)
{

    EXPECT_EQ(cbox1->text(),"hhh");
}

TEST_F(CheckBoxTest,checkbox1)
{

    EXPECT_NE(nullptr,cbox);
}
