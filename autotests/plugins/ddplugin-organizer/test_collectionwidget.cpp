// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectionwidget.h"
#include "view/collectiontitlebar.h"
#include "view/collectionview.h"
#include "mode/collectiondataprovider.h"
#include "interface/canvasinterface.h"

#include <QResizeEvent>
#include <QEvent>
#include <QEnterEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QPixmap>
#include <QTest>

#include "stubext.h"
#include "gtest/gtest.h"

using namespace ddplugin_organizer;

// Mock classes for testing

class MockCollectionDataProvider : public CollectionDataProvider
{
public:
    MockCollectionDataProvider(QObject *parent = nullptr) : CollectionDataProvider(parent) {}
    
    // 实现纯虚函数
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) override { Q_UNUSED(oldUrl); Q_UNUSED(newUrl); return QString(); }
    QString append(const QUrl &url) override { Q_UNUSED(url); return QString(); }
    QString prepend(const QUrl &url) override { Q_UNUSED(url); return QString(); }
    void insert(const QUrl &url, const QString &key, const int index) override { Q_UNUSED(url); Q_UNUSED(key); Q_UNUSED(index); }
    QString remove(const QUrl &url) override { Q_UNUSED(url); return QString(); }
    QString change(const QUrl &url) override { Q_UNUSED(url); return QString(); }
};

class UT_CollectionWidget : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new MockCollectionDataProvider();
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
        delete provider;

        stub.clear();
    }
    
    CollectionWidget* widget;
    MockCollectionDataProvider* provider;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionWidget, ConstructorTest)
{
    EXPECT_NE(widget, nullptr);
    EXPECT_EQ(widget->titleName(), QString("test-uuid"));
    EXPECT_FALSE(widget->renamable());
    EXPECT_FALSE(widget->closable());
    EXPECT_FALSE(widget->adjustable());
    EXPECT_NE(widget->view(), nullptr);
}

TEST_F(UT_CollectionWidget, SetTitleNameTest)
{
    widget->setTitleName("Test Collection");
    EXPECT_EQ(widget->titleName(), QString("Test Collection"));
    
    widget->setTitleName("");
    EXPECT_EQ(widget->titleName(), QString(""));
    
    widget->setTitleName("很长的收藏夹名称测试");
    EXPECT_EQ(widget->titleName(), QString("很长的收藏夹名称测试"));
}

TEST_F(UT_CollectionWidget, SetRenamableTest)
{
    widget->setRenamable(true);
    EXPECT_TRUE(widget->renamable());
    
    widget->setRenamable(false);
    EXPECT_FALSE(widget->renamable());
}

TEST_F(UT_CollectionWidget, SetClosableTest)
{
    widget->setClosable(true);
    EXPECT_TRUE(widget->closable());
    
    widget->setClosable(false);
    EXPECT_FALSE(widget->closable());
}

TEST_F(UT_CollectionWidget, SetAdjustableTest)
{
    widget->setAdjustable(true);
    EXPECT_TRUE(widget->adjustable());
    
    widget->setAdjustable(false);
    EXPECT_FALSE(widget->adjustable());
}

TEST_F(UT_CollectionWidget, SetCollectionSizeTest)
{
    widget->setCollectionSize(CollectionFrameSize::kSmall);
    EXPECT_EQ(widget->collectionSize(), CollectionFrameSize::kSmall);
    
    widget->setCollectionSize(CollectionFrameSize::kMiddle);
    EXPECT_EQ(widget->collectionSize(), CollectionFrameSize::kMiddle);
    
    widget->setCollectionSize(CollectionFrameSize::kLarge);
    EXPECT_EQ(widget->collectionSize(), CollectionFrameSize::kLarge);
    
    widget->setCollectionSize(CollectionFrameSize::kFree);
    EXPECT_EQ(widget->collectionSize(), CollectionFrameSize::kFree);
}

TEST_F(UT_CollectionWidget, ViewTest)
{
    CollectionView* view = widget->view();
    EXPECT_NE(view, nullptr);
    
    CollectionView* view2 = widget->view();
    EXPECT_EQ(view, view2);
}

TEST_F(UT_CollectionWidget, SetFreezeTest)
{
    widget->setFreeze(true);
    widget->setFreeze(false);
    
    widget->setFreeze(true);
    widget->setFreeze(true);
    widget->setFreeze(false);
}

TEST_F(UT_CollectionWidget, CacheSnapshotTest)
{
    widget->show();
    QTest::qWait(100);
    
    widget->cacheSnapshot();
    QTest::qWait(50);
    
    for (int i = 0; i < 3; ++i) {
        widget->cacheSnapshot();
        QTest::qWait(10);
    }
}

TEST_F(UT_CollectionWidget, SignalTest)
{
    bool closeRequested = false;
    bool adjustRequested = false;
    
    QObject::connect(widget, &CollectionWidget::sigRequestClose, [&](const QString &id) {
        closeRequested = true;
        EXPECT_EQ(id, QString("test-uuid"));
    });
    
    QObject::connect(widget, &CollectionWidget::sigRequestAdjustSizeMode, [&](const CollectionFrameSize &size) {
        adjustRequested = true;
    });
    
    widget->setTitleName("New Title");
    widget->setRenamable(true);
    widget->setClosable(true);
    widget->setAdjustable(true);
    
    widget->setCollectionSize(CollectionFrameSize::kLarge);
    QTest::qWait(100);
}

TEST_F(UT_CollectionWidget, ResizeEventTest)
{
    widget->show();
    QTest::qWait(50);
    
    QResizeEvent resizeEvent(QSize(300, 400), QSize(200, 300));
    QApplication::sendEvent(widget, &resizeEvent);
    QTest::qWait(10);
    
    QResizeEvent resizeEvent2(QSize(200, 300), QSize(300, 400));
    QApplication::sendEvent(widget, &resizeEvent2);
    QTest::qWait(10);
    
    for (int i = 0; i < 3; ++i) {
        QResizeEvent event(QSize(100 + i * 50, 200 + i * 50), QSize(100 + i * 50 - 10, 200 + i * 50 - 10));
        QApplication::sendEvent(widget, &event);
        QTest::qWait(5);
    }
}

TEST_F(UT_CollectionWidget, EventFilterTest)
{
    // bool eventFiltered = false;
    // stub.set_lamda(&QWidget::eventFilter, [&]() {
    //     eventFiltered = true;
    //     return false;
    // });
    
    QEvent event(QEvent::MouseButtonPress);
    bool result = widget->eventFilter(widget, &event);
    
    EXPECT_FALSE(result);
    
    for (int i = 0; i < 5; ++i) {
        QEvent testEvent(static_cast<QEvent::Type>(QEvent::User + i));
        widget->eventFilter(widget, &testEvent);
    }
}

TEST_F(UT_CollectionWidget, EnterEventTest)
{
    widget->show();
    QTest::qWait(50);
    
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(widget, &enterEvent);
#else
    QEnterEvent enterEvent(QPointF(10, 10), QPointF(0, 0), QPointF(10, 10));
    QApplication::sendEvent(widget, &enterEvent);
#endif
    
    QTest::qWait(10);
}

TEST_F(UT_CollectionWidget, LeaveEventTest)
{
    widget->show();
    QTest::qWait(50);
    
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(widget, &enterEvent);
#endif
    
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(widget, &leaveEvent);
    
    QTest::qWait(10);
}

TEST_F(UT_CollectionWidget, PaintEventTest)
{
    widget->show();
    QTest::qWait(50);
    
    QPaintEvent paintEvent(QRect(0, 0, 100, 100));
    QApplication::sendEvent(widget, &paintEvent);
    QTest::qWait(10);
    
    widget->setFreeze(true);
    widget->cacheSnapshot();
    QTest::qWait(50);
    
    QPaintEvent paintEvent2(QRect(0, 0, 200, 300));
    QApplication::sendEvent(widget, &paintEvent2);
    QTest::qWait(10);
}

TEST_F(UT_CollectionWidget, MultipleWidgetsTest)
{
    CollectionWidget* widget1 = new CollectionWidget("uuid1", provider);
    CollectionWidget* widget2 = new CollectionWidget("uuid2", provider);
    CollectionWidget* widget3 = new CollectionWidget("uuid3", provider);
    
    widget1->setTitleName("Widget 1");
    widget2->setTitleName("Widget 2");
    widget3->setTitleName("Widget 3");
    
    widget1->setRenamable(true);
    widget2->setClosable(true);
    widget3->setAdjustable(true);
    
    EXPECT_EQ(widget1->titleName(), QString("Widget 1"));
    EXPECT_EQ(widget2->titleName(), QString("Widget 2"));
    EXPECT_EQ(widget3->titleName(), QString("Widget 3"));
    
    EXPECT_TRUE(widget1->renamable());
    EXPECT_TRUE(widget2->closable());
    EXPECT_TRUE(widget3->adjustable());
    
    delete widget1;
    delete widget2;
    delete widget3;
}

TEST_F(UT_CollectionWidget, LifecycleTest)
{
    CollectionWidget* tempWidget = new CollectionWidget("temp-uuid", provider);
    EXPECT_NE(tempWidget, nullptr);
    
    tempWidget->setTitleName("Temp Widget");
    tempWidget->setRenamable(true);
    tempWidget->setClosable(true);
    tempWidget->setAdjustable(true);
    tempWidget->setCollectionSize(CollectionFrameSize::kLarge);
    
    EXPECT_EQ(tempWidget->titleName(), QString("Temp Widget"));
    EXPECT_TRUE(tempWidget->renamable());
    EXPECT_TRUE(tempWidget->closable());
    EXPECT_TRUE(tempWidget->adjustable());
    EXPECT_EQ(tempWidget->collectionSize(), CollectionFrameSize::kLarge);
    
    delete tempWidget;
}

TEST_F(UT_CollectionWidget, EdgeCasesTest)
{
    widget->setTitleName(QString());
    EXPECT_EQ(widget->titleName(), QString());
    
    widget->setTitleName("Special Characters: !@#$%^&*()_+{}|:<>?[]\\;'\",./");
    EXPECT_TRUE(widget->titleName().contains("!@#$%^&*()"));
    
    widget->setCollectionSize(static_cast<CollectionFrameSize>(999));
    
    widget->setFreeze(true);
    widget->setFreeze(false);
    widget->setFreeze(true);
    widget->setFreeze(false);
    
    CollectionWidget* emptyUuidWidget = new CollectionWidget("", provider);
    EXPECT_NE(emptyUuidWidget, nullptr);
    delete emptyUuidWidget;
}
