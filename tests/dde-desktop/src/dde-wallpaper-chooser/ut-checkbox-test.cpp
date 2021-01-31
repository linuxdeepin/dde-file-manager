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

#include "../dde-wallpaper-chooser/checkbox.h"

using namespace testing;

namespace {
      class CheckBoxTest : public Test {
      public:
          CheckBoxTest() : Test() {}

          virtual void SetUp() override
          {
              m_box = new CheckBox;
              m_box1 = new CheckBox("test");
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
    EXPECT_EQ(m_box1->text(), "test");
    EXPECT_NE(m_box, nullptr);
}

TEST_F(CheckBoxTest, key_pressevent_of_enter)
{
    m_box->setChecked(false);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    m_box->keyPressEvent(&event);
    EXPECT_EQ(m_box->isChecked(), true);

    m_box->setChecked(true);
    m_box->keyPressEvent(&event);
    EXPECT_FALSE(m_box->isChecked());
}
