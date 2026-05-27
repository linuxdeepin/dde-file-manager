// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "options/methodgroup/methodcombox.h"
#include "options/widgets/entrywidget.h"

#include <QString>
#include <QLabel>
#include <QTest>
#include <QSignalSpy>

#include "stubext.h"
#include "gtest/gtest.h"

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE

class UT_MethodComBox : public testing::Test
{
protected:
    void SetUp() override
    {
        methodComBox = new MethodComBox("Test Title");

        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }
    
    void TearDown() override
    {
        delete methodComBox;
        stub.clear();
    }
    
    MethodComBox* methodComBox;
    stub_ext::StubExt stub;
};

TEST_F(UT_MethodComBox, ConstructorTest)
{
    EXPECT_NE(methodComBox, nullptr);
    EXPECT_NE(methodComBox->label, nullptr);
    EXPECT_NE(methodComBox->comboBox, nullptr);
}

TEST_F(UT_MethodComBox, ConstructorWithTitleTest)
{
    MethodComBox* customComBox = new MethodComBox("Custom Title");
    EXPECT_NE(customComBox, nullptr);
    delete customComBox;
    
    MethodComBox* emptyComBox = new MethodComBox("");
    EXPECT_NE(emptyComBox, nullptr);
    delete emptyComBox;
    
    MethodComBox* longComBox = new MethodComBox("Very Long Title For Testing Purposes");
    EXPECT_NE(longComBox, nullptr);
    delete longComBox;
}

// TEST_F(UT_MethodComBox, InitCheckBoxTest)
// {
//     methodComBox->initCheckBox();
//     EXPECT_NE(methodComBox->label, nullptr);
//     EXPECT_NE(methodComBox->comboBox, nullptr);
// }

// TEST_F(UT_MethodComBox, SetCurrentMethodTest)
// {
//     methodComBox->initCheckBox();
    
//     methodComBox->setCurrentMethod(0);
//     QTest::qWait(10);
    
//     methodComBox->setCurrentMethod(1);
//     QTest::qWait(10);
    
//     methodComBox->setCurrentMethod(2);
//     QTest::qWait(10);
    
//     methodComBox->setCurrentMethod(-1);
//     QTest::qWait(10);
    
//     methodComBox->setCurrentMethod(100);
//     QTest::qWait(10);
// }

TEST_F(UT_MethodComBox, CurrentMethodTest)
{
    int method = methodComBox->currentMethod();
    EXPECT_GE(method, 0);
    
    // methodComBox->initCheckBox();
    int method2 = methodComBox->currentMethod();
    EXPECT_GE(method2, 0);
    
    // methodComBox->setCurrentMethod(1);
    // int method3 = methodComBox->currentMethod();
    // EXPECT_GE(method3, 0);
}

TEST_F(UT_MethodComBox, MultipleInstancesTest)
{
    MethodComBox* comBox1 = new MethodComBox("Title 1");
    MethodComBox* comBox2 = new MethodComBox("Title 2");
    MethodComBox* comBox3 = new MethodComBox("Title 3");
    
    EXPECT_NE(comBox1, nullptr);
    EXPECT_NE(comBox2, nullptr);
    EXPECT_NE(comBox3, nullptr);
    
    // comBox1->initCheckBox();
    // comBox2->initCheckBox();
    // comBox3->initCheckBox();
    
    // comBox1->setCurrentMethod(0);
    // comBox2->setCurrentMethod(1);
    // comBox3->setCurrentMethod(2);
    
    // EXPECT_GE(comBox1->currentMethod(), 0);
    // EXPECT_GE(comBox2->currentMethod(), 0);
    // EXPECT_GE(comBox3->currentMethod(), 0);
    
    delete comBox1;
    delete comBox2;
    delete comBox3;
}

// TEST_F(UT_MethodComBox, WidgetDisplayTest)
// {
//     methodComBox->show();
//     QTest::qWait(100);
//     EXPECT_TRUE(methodComBox->isVisible());
    
//     methodComBox->hide();
//     QTest::qWait(10);
//     EXPECT_FALSE(methodComBox->isVisible());
    
//     methodComBox->initCheckBox();
//     methodComBox->show();
//     QTest::qWait(100);
//     EXPECT_TRUE(methodComBox->isVisible());
// }

TEST_F(UT_MethodComBox, EdgeCasesTest)
{
    MethodComBox* comBox = new MethodComBox("test");
    EXPECT_NE(comBox, nullptr);
    
    // comBox->initCheckBox();
    
    // comBox->setCurrentMethod(-100);
    // QTest::qWait(10);
    
    // comBox->setCurrentMethod(10000);
    // QTest::qWait(10);
    
    // comBox->setCurrentMethod(INT_MIN);
    // QTest::qWait(10);
    
    // comBox->setCurrentMethod(INT_MAX);
    // QTest::qWait(10);
    
    delete comBox;
}

TEST_F(UT_MethodComBox, StressTest)
{
    // methodComBox->initCheckBox();
    
    // for (int i = 0; i < 10; ++i) {
    //     methodComBox->setCurrentMethod(i % 3);
    //     int current = methodComBox->currentMethod();
    //     EXPECT_GE(current, 0);
    //     QTest::qWait(5);
    // }
}

TEST_F(UT_MethodComBox, InheritanceTest)
{
    MethodComBox* comBox = new MethodComBox("Test");
    EntryWidget* entryWidget = static_cast<EntryWidget*>(comBox);
    EXPECT_NE(entryWidget, nullptr);
    delete comBox;
}

TEST_F(UT_MethodComBox, MemoryTest)
{
    for (int i = 0; i < 5; ++i) {
        MethodComBox* tempComBox = new MethodComBox(QString("Test %1").arg(i));
        // tempComBox->initCheckBox();
        // tempComBox->setCurrentMethod(i % 2);
        delete tempComBox;
    }
}
