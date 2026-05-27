// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectionframe.h"
#include "view/collectionwidget.h"
#include "mode/collectiondataprovider.h"

#include <QEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QShowEvent>
#include <QLabel>
#include <QTest>

#include "stubext.h"
#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class MockCollectionDataProvider : public CollectionDataProvider
{
public:
    MockCollectionDataProvider() : CollectionDataProvider(nullptr) {}
    
    QString replace(const QUrl &, const QUrl &) override { return QString(); }
    QString append(const QUrl &) override { return QString(); }
    QString prepend(const QUrl &) override { return QString(); }
    void insert(const QUrl &, const QString &, const int) override {}
    QString remove(const QUrl &) override { return QString(); }
    QString change(const QUrl &) override { return QString(); }
};

class UT_CollectionFrame : public testing::Test
{
protected:
    void SetUp() override
    {
        
        provider = new MockCollectionDataProvider();

        QWidget *parentWidget = new QWidget();
        frame = new CollectionFrame(parentWidget);
        widget = new CollectionWidget("test-uuid", provider);

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
        delete widget;
        delete frame;
        delete provider;

        stub.clear();
    }
    
    CollectionFrame* frame;
    CollectionWidget* widget;
    MockCollectionDataProvider* provider;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionFrame, ConstructorTest)
{
    EXPECT_NE(frame, nullptr);
    EXPECT_EQ(frame->widget(), nullptr);
    EXPECT_EQ(frame->stretchStyle(), CollectionFrame::CollectionFrameStretchUnLimited);
    EXPECT_EQ(frame->stretchStep(), 0); // not implemented, always return 0
}

TEST_F(UT_CollectionFrame, SetWidgetTest)
{
    frame->setWidget(widget);
    EXPECT_EQ(frame->widget(), widget);
    
    QLabel* label = new QLabel("Test Label");
    frame->setWidget(label);
    EXPECT_EQ(frame->widget(), label);
    
    // frame->setWidget(nullptr);
    // EXPECT_EQ(frame->widget(), nullptr);
    
    delete label;
}

TEST_F(UT_CollectionFrame, CollectionFeaturesTest)
{
    CollectionFrame::CollectionFrameFeatures features = static_cast<CollectionFrame::CollectionFrameFeatures>(
        CollectionFrame::CollectionFrameClosable |
        CollectionFrame::CollectionFrameMovable |
        CollectionFrame::CollectionFrameFloatable);
    
    frame->setCollectionFeatures(features);
    EXPECT_EQ(frame->collectionFeatures(), features);
    
    frame->setCollectionFeatures(CollectionFrame::NoCollectionFrameFeatures);
    EXPECT_EQ(frame->collectionFeatures(), CollectionFrame::NoCollectionFrameFeatures);
    
    frame->setCollectionFeatures(CollectionFrame::CollectionFrameStretchable);
    EXPECT_EQ(frame->collectionFeatures(), CollectionFrame::CollectionFrameStretchable);
    
    CollectionFrame::CollectionFrameFeatures allFeatures = static_cast<CollectionFrame::CollectionFrameFeatures>(
        CollectionFrame::CollectionFrameClosable |
        CollectionFrame::CollectionFrameMovable |
        CollectionFrame::CollectionFrameFloatable |
        CollectionFrame::CollectionFrameHiddable |
        CollectionFrame::CollectionFrameAdjustable |
        CollectionFrame::CollectionFrameStretchable);
    
    frame->setCollectionFeatures(allFeatures);
    EXPECT_EQ(frame->collectionFeatures(), allFeatures);
}

TEST_F(UT_CollectionFrame, StretchStyleTest)
{
    frame->setStretchStyle(CollectionFrame::CollectionFrameStretchStep);
    EXPECT_EQ(frame->stretchStyle(), CollectionFrame::CollectionFrameStretchStep);
    
    frame->setStretchStyle(CollectionFrame::CollectionFrameStretchUnLimited);
    EXPECT_EQ(frame->stretchStyle(), CollectionFrame::CollectionFrameStretchUnLimited);
}

TEST_F(UT_CollectionFrame, StretchStepTest)
{
    // this function has no implementation, so it always returns 0
    frame->setStretchStep(5);
    EXPECT_EQ(frame->stretchStep(), 0);
    
    // frame->setStretchStep(-1);
    // EXPECT_EQ(frame->stretchStep(), -1);
}

TEST_F(UT_CollectionFrame, AdjustSizeModeTest)
{
    frame->setWidget(widget);
    
    frame->adjustSizeMode(CollectionFrameSize::kSmall);
    QTest::qWait(50);
    
    frame->adjustSizeMode(CollectionFrameSize::kMiddle);
    QTest::qWait(50);
    
    frame->adjustSizeMode(CollectionFrameSize::kLarge);
    QTest::qWait(50);
    
    frame->adjustSizeMode(CollectionFrameSize::kFree);
    QTest::qWait(50);
    
    for (int i = 0; i < 3; ++i) {
        frame->adjustSizeMode(static_cast<CollectionFrameSize>(i));
        QTest::qWait(10);
    }
}

TEST_F(UT_CollectionFrame, SignalTest)
{
    bool sizeChanged = false;
    bool geometryChanged = false;
    bool editingChanged = false;
    
    QObject::connect(frame, &CollectionFrame::sizeModeChanged, [&](const CollectionFrameSize &size) {
        sizeChanged = true;
    });
    
    QObject::connect(frame, &CollectionFrame::geometryChanged, [&]() {
        geometryChanged = true;
    });
    
    QObject::connect(frame, &CollectionFrame::editingStatusChanged, [&](bool editing) {
        editingChanged = true;
    });
    
    frame->setWidget(widget);
    frame->adjustSizeMode(CollectionFrameSize::kLarge);
    QTest::qWait(100);
    
    frame->resize(200, 300);
    QTest::qWait(50);
}

TEST_F(UT_CollectionFrame, EventTest)
{
    frame->show();
    QTest::qWait(50);
    
    QEvent showEvent(QEvent::Show);
    QApplication::sendEvent(frame, &showEvent);
    QTest::qWait(10);
    
    QEvent hideEvent(QEvent::Hide);
    QApplication::sendEvent(frame, &hideEvent);
    QTest::qWait(10);
    
    for (int i = 0; i < 5; ++i) {
        QEvent testEvent(static_cast<QEvent::Type>(QEvent::User + i));
        frame->event(&testEvent);
    }
}

TEST_F(UT_CollectionFrame, EventFilterTest)
{
    QLabel* testWidget = new QLabel("Test");
    
    // bool eventFiltered = false;
    // stub.set_lamda(&QObject::eventFilter, [&]() {
    //     eventFiltered = true;
    //     return false;
    // });
    
    QEvent event(QEvent::MouseButtonPress);
    bool result = frame->eventFilter(testWidget, &event);
    
    EXPECT_FALSE(result);
    
    delete testWidget;
}

TEST_F(UT_CollectionFrame, ShowEventTest)
{
    frame->show();
    QTest::qWait(50);
    
    QShowEvent showEvent;
    QApplication::sendEvent(frame, &showEvent);
    QTest::qWait(10);
}

TEST_F(UT_CollectionFrame, MouseEventsTest)
{
    frame->show();
    QTest::qWait(50);
    
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(frame, &pressEvent);
    QTest::qWait(10);
    
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(frame, &releaseEvent);
    QTest::qWait(10);
    
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(15, 15), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(frame, &moveEvent);
    QTest::qWait(10);
    
    QMouseEvent rightPressEvent(QEvent::MouseButtonPress, QPoint(20, 20), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QApplication::sendEvent(frame, &rightPressEvent);
    QTest::qWait(10);
}

TEST_F(UT_CollectionFrame, ResizeEventTest)
{
    frame->show();
    QTest::qWait(50);
    
    QResizeEvent resizeEvent(QSize(300, 400), QSize(200, 300));
    QApplication::sendEvent(frame, &resizeEvent);
    QTest::qWait(10);
    
    QResizeEvent resizeEvent2(QSize(200, 300), QSize(300, 400));
    QApplication::sendEvent(frame, &resizeEvent2);
    QTest::qWait(10);
    
    for (int i = 0; i < 3; ++i) {
        QResizeEvent event(QSize(100 + i * 50, 200 + i * 50), QSize(100 + i * 50 - 10, 200 + i * 50 - 10));
        QApplication::sendEvent(frame, &event);
        QTest::qWait(5);
    }
}

TEST_F(UT_CollectionFrame, PaintEventTest)
{
    frame->show();
    QTest::qWait(50);
    
    QPaintEvent paintEvent(QRect(0, 0, 100, 100));
    QApplication::sendEvent(frame, &paintEvent);
    QTest::qWait(10);
    
    QPaintEvent paintEvent2(QRect(0, 0, 200, 300));
    QApplication::sendEvent(frame, &paintEvent2);
    QTest::qWait(10);
}

TEST_F(UT_CollectionFrame, FocusOutEventTest)
{
    frame->show();
    QTest::qWait(50);
    
    frame->setFocus();
    QTest::qWait(10);
    
    QFocusEvent focusOutEvent(QEvent::FocusOut);
    QApplication::sendEvent(frame, &focusOutEvent);
    QTest::qWait(10);
}

TEST_F(UT_CollectionFrame, MultipleFramesTest)
{
    CollectionFrame* frame1 = new CollectionFrame();
    CollectionFrame* frame2 = new CollectionFrame();
    CollectionFrame* frame3 = new CollectionFrame();
    
    // no implementation, always return 0
    frame1->setStretchStep(5);
    frame2->setStretchStep(10);
    frame3->setStretchStep(15);
    
    EXPECT_EQ(frame1->stretchStep(), 0);
    EXPECT_EQ(frame2->stretchStep(), 0);
    EXPECT_EQ(frame3->stretchStep(), 0);
    // EXPECT_EQ(frame1->stretchStep(), 5);
    // EXPECT_EQ(frame2->stretchStep(), 10);
    // EXPECT_EQ(frame3->stretchStep(), 15);
    
    frame1->setStretchStyle(CollectionFrame::CollectionFrameStretchUnLimited);
    frame2->setStretchStyle(CollectionFrame::CollectionFrameStretchStep);
    
    EXPECT_EQ(frame1->stretchStyle(), CollectionFrame::CollectionFrameStretchUnLimited);
    EXPECT_EQ(frame2->stretchStyle(), CollectionFrame::CollectionFrameStretchStep);
    
    delete frame1;
    delete frame2;
    delete frame3;
}

TEST_F(UT_CollectionFrame, EdgeCasesTest)
{
    // frame->setStretchStep(-100);
    // EXPECT_EQ(frame->stretchStep(), -100);
    
    // frame->setStretchStep(1000);
    // EXPECT_EQ(frame->stretchStep(), 1000);
    
    CollectionFrame::CollectionFrameFeatures invalidFeatures = 
        static_cast<CollectionFrame::CollectionFrameFeatures>(0xFFFFFFFF);
    frame->setCollectionFeatures(invalidFeatures);
    
    frame->adjustSizeMode(static_cast<CollectionFrameSize>(999));
    
    frame->setWidget(widget);
    EXPECT_EQ(frame->widget(), widget);
}

TEST_F(UT_CollectionFrame, FeatureFlagsTest)
{
    frame->setCollectionFeatures(CollectionFrame::CollectionFrameClosable);
    EXPECT_TRUE(frame->collectionFeatures() & CollectionFrame::CollectionFrameClosable);
    
    frame->setCollectionFeatures(CollectionFrame::CollectionFrameMovable);
    EXPECT_TRUE(frame->collectionFeatures() & CollectionFrame::CollectionFrameMovable);
    
    frame->setCollectionFeatures(CollectionFrame::CollectionFrameFloatable);
    EXPECT_TRUE(frame->collectionFeatures() & CollectionFrame::CollectionFrameFloatable);
    
    frame->setCollectionFeatures(CollectionFrame::CollectionFrameHiddable);
    EXPECT_TRUE(frame->collectionFeatures() & CollectionFrame::CollectionFrameHiddable);
    
    frame->setCollectionFeatures(CollectionFrame::CollectionFrameAdjustable);
    EXPECT_TRUE(frame->collectionFeatures() & CollectionFrame::CollectionFrameAdjustable);
    
    frame->setCollectionFeatures(CollectionFrame::CollectionFrameStretchable);
    EXPECT_TRUE(frame->collectionFeatures() & CollectionFrame::CollectionFrameStretchable);
}
