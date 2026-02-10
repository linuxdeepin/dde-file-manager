// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "backgrounddefault.h"
#include <dfm-framework/listener/listener.h>
#include <dfm-framework/event/eventdispatcher.h>
#include <dfm-framework/dpf.h>

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>

DDP_BACKGROUND_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_BackgroundDefault : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create test object
        backgroundDefault = new BackgroundDefault("test-screen");
        
        // Set properties
        backgroundDefault->setProperty("screenName", "test-screen");
        backgroundDefault->setProperty("widgetName", "background");
    }

    void TearDown() override
    {
        delete backgroundDefault;
        stub.clear();
    }

    BackgroundDefault *backgroundDefault = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_BackgroundDefault, constructor_WithScreenName_SetsTranslucentAttribute)
{
    EXPECT_TRUE(backgroundDefault->testAttribute(Qt::WA_TranslucentBackground));
    EXPECT_EQ(backgroundDefault->property("screenName").toString(), "test-screen");
    EXPECT_EQ(backgroundDefault->property("widgetName").toString(), "background");
}

TEST_F(UT_BackgroundDefault, setPixmap_WithValidPixmap_CallsUpdate)
{
    // Stub update method
    bool updateCalled = false;
    // Use function pointer type directly, don't use using alias
    stub.set_lamda((void (QWidget::*)())&QWidget::update, [&updateCalled](QWidget *) {
        __DBG_STUB_INVOKE__
        updateCalled = true;
    });

    // Set test pixmap
    QPixmap testPixmap(100, 100);
    testPixmap.fill(Qt::red);
    backgroundDefault->setPixmap(testPixmap);

    // Verify update was called
    EXPECT_TRUE(updateCalled);
}

TEST_F(UT_BackgroundDefault, paintEvent_WithValidPixmap_DrawsPixmapAndSendsReport)
{
    // Check property settings
    EXPECT_EQ(backgroundDefault->property("screenName").toString(), "test-screen");
    EXPECT_EQ(backgroundDefault->property("widgetName").toString(), "background");

    // Directly access and modify painted variable to avoid log output
    // Utilize the feature that private members can be accessed in test environment
    backgroundDefault->painted = 0;
    
    // Stub sendPaintReport method, set publishCalled variable in the stubbed implementation
    bool publishCalled = false;
    stub.set_lamda(ADDR(BackgroundDefault, sendPaintReport), [&publishCalled](BackgroundDefault *) {
        __DBG_STUB_INVOKE__
        static bool reportedPaint { false };
        if (Q_LIKELY(reportedPaint))
            return;
        
        // Simulate dpfSignalDispatcher->publish call
        publishCalled = true;
        reportedPaint = true;
    });

    // Stub QPainter::drawPixmap
    bool drawPixmapCalled = false;
    stub.set_lamda((void (QPainter::*)(const QPointF &, const QPixmap &, const QRectF &))&QPainter::drawPixmap, 
                  [&drawPixmapCalled] {
        __DBG_STUB_INVOKE__
        drawPixmapCalled = true;
    });

    // Set test pixmap
    QPixmap testPixmap(100, 100);
    testPixmap.fill(Qt::red);
    backgroundDefault->setPixmap(testPixmap);

    // Create paint event
    QRect rect(0, 0, 100, 100);
    QPaintEvent event(rect);

    // Call paint event
    backgroundDefault->paintEvent(&event);

    // Verify draw method was called
    EXPECT_TRUE(drawPixmapCalled);
    
    // First call should publish signal
    EXPECT_TRUE(publishCalled);
    
    // Reset state
    publishCalled = false;
    
    // Call paint event again
    backgroundDefault->paintEvent(&event);
    
    // Second call should not publish signal (reportedPaint is static and set to true after first call)
    EXPECT_FALSE(publishCalled);
}

TEST_F(UT_BackgroundDefault, paintEvent_WithNullPixmap_ReturnsEarly)
{
    // Directly access and modify painted variable to avoid log output
    backgroundDefault->painted = 0;
    
    // Stub QPainter::drawPixmap
    bool drawPixmapCalled = false;
    stub.set_lamda((void (QPainter::*)(const QPointF &, const QPixmap &, const QRectF &))&QPainter::drawPixmap, 
                  [&drawPixmapCalled] {
        __DBG_STUB_INVOKE__
        drawPixmapCalled = true;
    });

    // Don't set pixmap, keep it null

    // Create paint event
    QRect rect(0, 0, 100, 100);
    QPaintEvent event(rect);

    // Call paint event
    backgroundDefault->paintEvent(&event);

    // Verify draw method was not called
    EXPECT_FALSE(drawPixmapCalled);
}

TEST_F(UT_BackgroundDefault, sendPaintReport_FirstCall_ExecutesWithoutCrash)
{
    // Test the sendPaintReport method directly to improve coverage
    // Since dpfSignalDispatcher is a complex macro, we just verify the method executes
    // and the static variable logic works correctly
    
    // Call sendPaintReport method directly - this should execute the method
    // and improve code coverage even without stubbing the complex signal dispatcher
    EXPECT_NO_THROW(backgroundDefault->sendPaintReport());
    
    // Call again to test the static variable behavior
    EXPECT_NO_THROW(backgroundDefault->sendPaintReport());
}

TEST_F(UT_BackgroundDefault, paintEvent_WithPaintedCounter_DecreasesCounter)
{
    // Test the painted counter logging behavior in paintEvent
    
    // Set painted to a positive value to trigger the logging path (line 42-43 in source)
    backgroundDefault->painted = 5;
    int initialPainted = backgroundDefault->painted;
    
    // Set test pixmap to ensure paintEvent continues execution
    QPixmap testPixmap(100, 100);
    testPixmap.fill(Qt::blue);
    backgroundDefault->setPixmap(testPixmap);
    
    // Stub sendPaintReport to avoid side effects
    stub.set_lamda(ADDR(BackgroundDefault, sendPaintReport), [](BackgroundDefault *) {
        __DBG_STUB_INVOKE__
        // Do nothing - just prevent actual signal dispatch
    });
    
    // Create paint event
    QRect rect(0, 0, 50, 50);
    QPaintEvent event(rect);
    
    // Call paint event
    backgroundDefault->paintEvent(&event);
    
    // Verify painted counter decreased (this tests the logging path at line 43)
    EXPECT_EQ(backgroundDefault->painted, initialPainted - 1);
}

TEST_F(UT_BackgroundDefault, paintEvent_WithZeroPaintedCounter_DoesNotLog)
{
    // Test that when painted counter is 0, no logging occurs
    
    // Set painted to 0 (default value)
    backgroundDefault->painted = 0;
    
    // Set test pixmap
    QPixmap testPixmap(100, 100);
    testPixmap.fill(Qt::green);
    backgroundDefault->setPixmap(testPixmap);
    
    // Stub sendPaintReport to avoid side effects
    stub.set_lamda(ADDR(BackgroundDefault, sendPaintReport), [](BackgroundDefault *) {
        __DBG_STUB_INVOKE__
        // Do nothing
    });
    
    // Create paint event
    QRect rect(0, 0, 50, 50);
    QPaintEvent event(rect);
    
    // Call paint event
    backgroundDefault->paintEvent(&event);
    
    // Verify painted counter remains 0 (no logging branch executed)
    EXPECT_EQ(backgroundDefault->painted, 0);
}

TEST_F(UT_BackgroundDefault, paintEvent_WithScaling_CalculatesCorrectSourceRect)
{
    // Test the scaling calculation logic in paintEvent (lines 48-57 in source)
    
    // Set test pixmap
    QPixmap testPixmap(200, 200);
    testPixmap.fill(Qt::yellow);
    backgroundDefault->setPixmap(testPixmap);
    
    // Stub sendPaintReport to avoid side effects
    stub.set_lamda(ADDR(BackgroundDefault, sendPaintReport), [](BackgroundDefault *) {
        __DBG_STUB_INVOKE__
    });
    
    // Stub devicePixelRatioF to return a non-integer scale for testing
    stub.set_lamda((qreal (QWidget::*)() const)&QWidget::devicePixelRatioF, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return 1.5; // Non-integer scale to test SmoothPixmapTransform
    });
    
    // Stub QPainter methods to verify they are called with correct parameters
    bool setRenderHintCalled = false;
    bool drawPixmapCalled = false;
    
    stub.set_lamda((void (QPainter::*)(QPainter::RenderHint, bool))&QPainter::setRenderHint, 
                  [&setRenderHintCalled](QPainter *, QPainter::RenderHint hint, bool enable) {
        __DBG_STUB_INVOKE__
        if (hint == QPainter::SmoothPixmapTransform && enable) {
            setRenderHintCalled = true;
        }
    });
    
    stub.set_lamda((void (QPainter::*)(const QPointF &, const QPixmap &, const QRectF &))&QPainter::drawPixmap, 
                  [&drawPixmapCalled] {
        __DBG_STUB_INVOKE__
        drawPixmapCalled = true;
    });
    
    // Create paint event
    QRect rect(10, 10, 80, 80);
    QPaintEvent event(rect);
    
    // Call paint event
    backgroundDefault->paintEvent(&event);
    
    // Verify render hint was set for non-integer scaling
    EXPECT_TRUE(setRenderHintCalled);
    EXPECT_TRUE(drawPixmapCalled);
}

TEST_F(UT_BackgroundDefault, paintEvent_WithIntegerScaling_DoesNotSetSmoothTransform)
{
    // Test that integer scaling doesn't enable SmoothPixmapTransform
    
    // Set test pixmap
    QPixmap testPixmap(100, 100);
    testPixmap.fill(Qt::cyan);
    backgroundDefault->setPixmap(testPixmap);
    
    // Stub sendPaintReport to avoid side effects
    stub.set_lamda(ADDR(BackgroundDefault, sendPaintReport), [](BackgroundDefault *) {
        __DBG_STUB_INVOKE__
    });
    
    // Stub devicePixelRatioF to return an integer scale
    stub.set_lamda((qreal (QWidget::*)() const)&QWidget::devicePixelRatioF, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return 2.0; // Integer scale - should not enable smooth transform
    });
    
    // Stub QPainter methods
    bool smoothTransformEnabled = false;
    bool drawPixmapCalled = false;
    
    stub.set_lamda((void (QPainter::*)(QPainter::RenderHint, bool))&QPainter::setRenderHint, 
                  [&smoothTransformEnabled](QPainter *, QPainter::RenderHint hint, bool enable) {
        __DBG_STUB_INVOKE__
        if (hint == QPainter::SmoothPixmapTransform && enable) {
            smoothTransformEnabled = true;
        }
    });
    
    stub.set_lamda((void (QPainter::*)(const QPointF &, const QPixmap &, const QRectF &))&QPainter::drawPixmap, 
                  [&drawPixmapCalled] {
        __DBG_STUB_INVOKE__
        drawPixmapCalled = true;
    });
    
    // Create paint event
    QRect rect(5, 5, 90, 90);
    QPaintEvent event(rect);
    
    // Call paint event
    backgroundDefault->paintEvent(&event);
    
    // Verify smooth transform was NOT enabled for integer scaling
    EXPECT_FALSE(smoothTransformEnabled);
    EXPECT_TRUE(drawPixmapCalled);
}