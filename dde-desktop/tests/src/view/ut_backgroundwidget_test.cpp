#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <DWindowManagerHelper>
#include <QScopedPointer>
#include <QGuiApplication>
#include <QScreen>
#include <QTest>

#define private public
#define protected public

#include "view/backgroundwidget.h"
#include "view/canvasgridview.h"
#include "screen/screenhelper.h"

using namespace testing;
namespace  {
    class BackgroundWidgetTest : public Test
    {
    public:
        BackgroundWidgetTest():Test()
        {
            p_widget = nullptr;
            primaryScreen = ScreenMrg->primaryScreen();
        }

        virtual void SetUp() override {
            p_widget = new BackgroundWidget;
        }

        virtual void TearDown() override {
            delete  p_widget;
        }

        BackgroundWidget * p_widget;
        ScreenPointer primaryScreen;
    };
    class MyBackgroundWidget : public BackgroundWidget
    {
    public:
        MyBackgroundWidget() : BackgroundWidget(){}
    protected:
        void paintEvent(QPaintEvent *e) override{   //override form BackgroundWidget
            BackgroundWidget::paintEvent(e);
            paintEventCalled = true;
        }
    public:
        bool paintEventCalled = false;
        char other[7];
    };
}

TEST_F(BackgroundWidgetTest, create_object)
{
    ASSERT_NE(p_widget, nullptr);
}

TEST_F(BackgroundWidgetTest, set_pixmap)
{
    QPixmap pixmap(200,200);
    pixmap.fill(Qt::green);
    p_widget->setPixmap(pixmap);
    ASSERT_EQ(pixmap, p_widget->m_pixmap);
    ASSERT_EQ(pixmap, p_widget->m_noScalePixmap);
    ASSERT_EQ(1, p_widget->m_noScalePixmap.devicePixelRatio());
}
TEST_F(BackgroundWidgetTest, call_paintEvent)
{
    //reference from:qt/src/qtbase/tests/auto/widgets/widgets/qtabwidget
    MyBackgroundWidget w;
    w.paintEventCalled = false;
    w.show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(&w));
    // Wait for extra paint events that happen at least on macOS
    QTest::qWait(1000);
    ASSERT_EQ(w.paintEventCalled, true);
}

TEST_F(BackgroundWidgetTest, set_view)
{
    QSharedPointer<CanvasGridView> v(new CanvasGridView(primaryScreen->name()));
    p_widget->setView(v);
    ASSERT_EQ(v->parent(), p_widget);
    ASSERT_EQ(p_widget->m_view, v);
}

TEST_F(BackgroundWidgetTest, get_pixmap)
{
    QPixmap pixmap(200,200);
    pixmap.fill(Qt::red);
    p_widget->setPixmap(pixmap);
    QPixmap getPixmap = p_widget->pixmap();
    ASSERT_EQ(pixmap, getPixmap);
}
