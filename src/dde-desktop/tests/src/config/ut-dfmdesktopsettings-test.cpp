#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <QEvent>

using namespace testing;

#include "config/dfmdesktopsettings.h"
#include "config/profile.h"

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
}

TEST(ProfileTest, profile)
{
    Profile* file = new Profile;//空文件,空构造
    delete  file;
}
