#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include <presenter/dfmsocketinterface.h>
#include <presenter/dfmsocketinterface_p.h>
#include "dtkwidget_global.h"

TEST(dfmsocketinterfaceTest, test_getProcIdByName)
{
    DFMSocketInterface inter;
    EXPECT_TRUE(-1 == getProcIdByName("ahdha"));
    EXPECT_TRUE(1 == getProcIdByName("init"));
}
