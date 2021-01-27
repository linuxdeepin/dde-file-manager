#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QCoreApplication>
#include <QScreen>

#define private public
#define protected public

#include "../dde-desktop/deventfilter.h"

using namespace testing;
namespace  {
        class DEventFilterTest : public Test {
        public:
            DEventFilterTest() : Test() {}

            virtual void SetUp() override {
                m_eventfilter = new DEventFilter;
            }

            virtual void TearDown() override{
                delete m_eventfilter;
            }

            DEventFilter* m_eventfilter{nullptr};
        };
}


TEST_F(DEventFilterTest, test_eventfileter)
{
    CanvasGridView* view = new CanvasGridView("HDMI-0");
    QEvent* event = new QEvent(QEvent::DragEnter);

//    m_eventfilter->eventFilter(view, event);
//    bool bjudge = CanvasGridView::m_flag.load(std::memory_order_release);
//    EXPECT_EQ(bjudge, true);

    QEvent* event1 = new QEvent(QEvent::Hide);
    bool ret;
    ret = m_eventfilter->eventFilter(view, event1);
    EXPECT_EQ(ret, false);
    delete view;
    delete event;
    delete event1;
}
