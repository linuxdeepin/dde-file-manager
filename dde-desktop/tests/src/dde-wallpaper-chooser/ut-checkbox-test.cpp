#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include <QFileInfo>
#include <QThread>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QEventLoop>
#include <QObject>
#include <QTimer>

#define private public

#include "../../dde-wallpaper-chooser/checkbox.h"

using namespace testing;

namespace {
      class CheckBoxTest : public Test {
      public:
          CheckBoxTest() : Test() {}

          virtual void SetUp() override
          {
              m_box = new CheckBox;
              m_box1 = new CheckBox("kkk");
          }

          virtual void TearDown() override
          {
              delete m_box;
              delete m_box1;
          }

          CheckBox *m_box = nullptr;
          CheckBox *m_box1 = nullptr;
      };
}
TEST_F(CheckBoxTest, checkbox)
{
    EXPECT_EQ(m_box1->text(), "kkk");
    EXPECT_NE(m_box, nullptr);
}

TEST_F(CheckBoxTest, key_pressevent_of_enter)
{
    QTimer timer;
    timer.start(2000);
    bool bjudge = false;
    QTest::keyPress(m_box, Qt::Key_Enter);
    QEventLoop loop;
    QObject::connect(m_box, &CheckBox::clicked, &loop, [&](){
        bjudge = true;
        loop.exit();
    });
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
        loop.exit();
    });
    loop.exec();
    EXPECT_TRUE(bjudge);
}
