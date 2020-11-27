#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include <QEventLoop>
#include <QObject>
#include <QTimer>

#define private public
#define protected public

#include "../dde-wallpaper-chooser/button.h"

using namespace testing;

namespace  {
     class ButtonTest : public Test {
     public:
         ButtonTest() : Test()
         {

         }

         virtual void SetUp() override
         {
             m_button = new Button;
         }

         virtual void TearDown() override
         {
             delete m_button;
         }

         Button *m_button = nullptr;
     };
}

TEST_F(ButtonTest, get_buttonsize)
{
    QSize size = m_button->size();
    EXPECT_EQ(size.width(), 160);
    EXPECT_EQ(size.height(), 36);
}

TEST_F(ButtonTest, key_pressevent_of_space)
{
    QTimer timer;
    timer.start(100);
    bool bjudge = false;
    QObject::connect(m_button, &Button::clicked, m_button, [&]{
        bjudge = true;
    });
    QObject::connect(&timer, &QTimer::timeout, m_button, [&]{
        timer.stop();
    });
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    m_button->keyPressEvent(&event);
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();
    EXPECT_TRUE(bjudge);
}
