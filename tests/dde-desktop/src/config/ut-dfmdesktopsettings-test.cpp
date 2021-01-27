#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <QEvent>
#include <QSettings>

#include "config/dfmdesktopsettings.h"
#include "config/profile.h"
#include "canvasgridview.h"
#include "stubext.h"

using namespace testing;
namespace  {
        class DFMDesktopSettingsTest : public Test{
        public:
            DFMDesktopSettingsTest() : Test() {}

            virtual void SetUp() override {
                m_set = new DFMDesktopSettings("test.txt");
            }

            virtual void TearDown() override {
                delete m_set;
            }

            DFMDesktopSettings* m_set = nullptr;
        };
}

TEST_F(DFMDesktopSettingsTest, test_event)
{
    QEvent* event1 = new QEvent(QEvent::Hide);
    m_set->event(event1);

    QEvent* event2 = new QEvent(QEvent::UpdateRequest);
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(QSettings, sync), [](){return;});
    bool ret = m_set->event(event2);
    EXPECT_TRUE(ret);
    delete event1;
    delete event2;
}

TEST(ProfileTest, profile)
{
    Profile* file = new Profile;//空文件,空构造
    delete  file;
}
