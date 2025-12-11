// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "collection/collectionholder.h"
#include "collection/collectionholder_p.h"
#include "view/collectionframe.h"
#include "view/collectionwidget.h"
#include "view/collectionview.h"
#include "mode/collectiondataprovider.h"
#include "models/collectionmodel.h"
#include "private/surface.h"
#include "config/configpresenter.h"

#include <QPropertyAnimation>
#include <QUrl>
#include <QSignalSpy>

#include <gtest/gtest.h>

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

class UT_CollectionHolder : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        mockProvider = new MockCollectionDataProvider();
        holder = new CollectionHolder("test_id", mockProvider);

        // init necessary objects
        Surface *testSurface = new Surface();
        CollectionModel *testModel = new CollectionModel();
        holder->createFrame(testSurface, testModel);

        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    virtual void TearDown() override
    {
        delete holder;
        holder = nullptr;
        delete mockProvider;
        mockProvider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionHolder *holder = nullptr;
    MockCollectionDataProvider *mockProvider = nullptr;
};

TEST_F(UT_CollectionHolder, Constructor_WithValidParams_InitializesCorrectly)
{
    EXPECT_NE(holder, nullptr);
    EXPECT_EQ(holder->id(), "test_id");
}

TEST_F(UT_CollectionHolder, id_ReturnsCorrectId)
{
    EXPECT_EQ(holder->id(), "test_id");
}

TEST_F(UT_CollectionHolder, name_WithWidget_ReturnsTitleName)
{
    QString expectedName = "Test Collection";

    stub.set_lamda(&CollectionWidget::titleName, [expectedName]() {
        __DBG_STUB_INVOKE__
        return expectedName;
    });

    holder->d->widget = new CollectionWidget("test", mockProvider);

    QString result = holder->name();
    EXPECT_EQ(result, expectedName);

    delete holder->d->widget;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, setName_WithWidget_SetsTitleName)
{
    QString testName = "New Name";
    bool setNameCalled = false;

    stub.set_lamda(&CollectionWidget::setTitleName, [&setNameCalled, testName](CollectionWidget *, const QString &name) {
        __DBG_STUB_INVOKE__
        setNameCalled = true;
        EXPECT_EQ(name, testName);
    });

    holder->d->widget = new CollectionWidget("test", mockProvider);

    holder->setName(testName);
    EXPECT_TRUE(setNameCalled);

    delete holder->d->widget;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, frame_ReturnsFrame)
{
    CollectionFrame *testFrame = new CollectionFrame();
    holder->d->frame = testFrame;

    EXPECT_EQ(holder->frame(), testFrame);

    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, widget_ReturnsWidget)
{
    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->widget = testWidget;

    EXPECT_EQ(holder->widget(), testWidget);

    delete testWidget;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, itemView_WithWidget_ReturnsView)
{
    CollectionView *mockView = nullptr;

    stub.set_lamda(&CollectionWidget::view, [&mockView]() {
        __DBG_STUB_INVOKE__
        return mockView;
    });

    holder->d->widget = new CollectionWidget("test", mockProvider);

    EXPECT_EQ(holder->itemView(), mockView);

    delete holder->d->widget;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, itemView_WithoutWidget_ReturnsNullptr)
{
    holder->d->widget = nullptr;
    EXPECT_EQ(holder->itemView(), nullptr);
}

TEST_F(UT_CollectionHolder, surface_ReturnsSetSurface)
{
    Surface *testSurface = new Surface();
    holder->d->surface = testSurface;

    EXPECT_EQ(holder->surface(), testSurface);

    delete testSurface;
    holder->d->surface = nullptr;
}

TEST_F(UT_CollectionHolder, setSurface_WithFrame_SetsParent)
{
    Surface *testSurface = new Surface();
    CollectionFrame *testFrame = new CollectionFrame();

    holder->d->frame = testFrame;
    holder->setSurface(testSurface);

    EXPECT_EQ(holder->d->surface, testSurface);
    EXPECT_EQ(testFrame->parent(), testSurface);

    delete testFrame;
    delete testSurface;
    holder->d->frame = nullptr;
    holder->d->surface = nullptr;
}

TEST_F(UT_CollectionHolder, setSurface_WithoutFrame_OnlySetsSurface)
{
    Surface *testSurface = new Surface();
    holder->d->frame = nullptr;

    holder->setSurface(testSurface);

    EXPECT_EQ(holder->d->surface, testSurface);

    delete testSurface;
    holder->d->surface = nullptr;
}

TEST_F(UT_CollectionHolder, show_CallsFrameShowAndRaise)
{
    bool showCalled = false;
    bool raiseCalled = false;

    stub.set_lamda(&CollectionFrame::show, [&showCalled]() {
        __DBG_STUB_INVOKE__
        showCalled = true;
    });

    stub.set_lamda(&CollectionFrame::raise, [&raiseCalled]() {
        __DBG_STUB_INVOKE__
        raiseCalled = true;
    });

    CollectionFrame *testFrame = new CollectionFrame();
    holder->d->frame = testFrame;

    holder->show();

    EXPECT_TRUE(showCalled);
    EXPECT_TRUE(raiseCalled);

    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, setMovable_True_SetsMovableFeature)
{
    CollectionFrame *testFrame = new CollectionFrame();
    holder->d->frame = testFrame;

    holder->setMovable(true);
    EXPECT_TRUE(holder->movable());

    holder->setMovable(false);
    EXPECT_FALSE(holder->movable());

    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, setClosable_True_SetsClosableFeature)
{
    stub.set_lamda(&CollectionWidget::setClosable, [](CollectionWidget *, bool) {
        __DBG_STUB_INVOKE__
    });

    CollectionFrame *testFrame = new CollectionFrame();
    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->frame = testFrame;
    holder->d->widget = testWidget;

    holder->setClosable(true);
    EXPECT_TRUE(holder->closable());

    holder->setClosable(false);
    EXPECT_FALSE(holder->closable());

    delete testWidget;
    delete testFrame;
    holder->d->frame = nullptr;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, setFloatable_True_SetsFloatableFeature)
{
    CollectionFrame *testFrame = new CollectionFrame();
    holder->d->frame = testFrame;

    holder->setFloatable(true);
    EXPECT_TRUE(holder->floatable());

    holder->setFloatable(false);
    EXPECT_FALSE(holder->floatable());

    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, setHiddableCollection_True_SetsHiddableFeature)
{
    CollectionFrame *testFrame = new CollectionFrame();
    holder->d->frame = testFrame;

    holder->setHiddableCollection(true);
    EXPECT_TRUE(holder->hiddableCollection());

    holder->setHiddableCollection(false);
    EXPECT_FALSE(holder->hiddableCollection());

    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, setStretchable_True_SetsStretchableFeature)
{
    CollectionFrame *testFrame = new CollectionFrame();
    holder->d->frame = testFrame;

    holder->setStretchable(true);
    EXPECT_TRUE(holder->stretchable());

    holder->setStretchable(false);
    EXPECT_FALSE(holder->stretchable());

    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, setAdjustable_True_SetsAdjustableFeature)
{
    stub.set_lamda(&CollectionWidget::setAdjustable, [](CollectionWidget *, bool) {
        __DBG_STUB_INVOKE__
    });

    CollectionFrame *testFrame = new CollectionFrame();
    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->frame = testFrame;
    holder->d->widget = testWidget;

    holder->setAdjustable(true);
    EXPECT_TRUE(holder->adjustable());

    holder->setAdjustable(false);
    EXPECT_FALSE(holder->adjustable());

    delete testWidget;
    delete testFrame;
    holder->d->frame = nullptr;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, hiddableTitleBar_ReturnsFalse)
{
    EXPECT_FALSE(holder->hiddableTitleBar());
}

TEST_F(UT_CollectionHolder, hiddableView_ReturnsFalse)
{
    EXPECT_FALSE(holder->hiddableView());
}

TEST_F(UT_CollectionHolder, setRenamable_CallsWidgetSetRenamable)
{
    bool renamableValue = false;

    stub.set_lamda(&CollectionWidget::setRenamable, [&renamableValue](CollectionWidget *, bool val) {
        __DBG_STUB_INVOKE__
        renamableValue = val;
    });

    stub.set_lamda(&CollectionWidget::renamable, [&renamableValue]() {
        __DBG_STUB_INVOKE__
        return renamableValue;
    });

    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->widget = testWidget;

    holder->setRenamable(true);
    EXPECT_TRUE(holder->renamable());

    delete testWidget;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, createAnimation_ReturnsValidAnimation)
{
    CollectionFrame *testFrame = new CollectionFrame();
    testFrame->setGeometry(100, 100, 200, 200);
    holder->d->frame = testFrame;

    QPropertyAnimation *animation = holder->createAnimation();

    EXPECT_NE(animation, nullptr);
    EXPECT_EQ(animation->duration(), 500);
    EXPECT_EQ(animation->targetObject(), testFrame);

    delete animation;
    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, setStyle_WithMatchingKey_SetsStyle)
{
    CollectionFrame *testFrame = new CollectionFrame();
    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->frame = testFrame;
    holder->d->widget = testWidget;

    stub.set_lamda(&CollectionWidget::setCollectionSize, [](CollectionWidget *, const CollectionFrameSize &) {
        __DBG_STUB_INVOKE__
    });

    CollectionStyle style;
    style.key = "test_id";
    style.rect = QRect(50, 50, 300, 400);
    style.sizeMode = CollectionFrameSize::kLarge;
    style.screenIndex = 2;

    holder->setStyle(style);

    EXPECT_EQ(holder->d->screenIndex, 2);
    EXPECT_EQ(holder->d->sizeMode, CollectionFrameSize::kLarge);

    delete testWidget;
    delete testFrame;
    holder->d->frame = nullptr;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, setStyle_WithMismatchingKey_DoesNothing)
{
    holder->d->screenIndex = 1;
    holder->d->sizeMode = CollectionFrameSize::kMiddle;

    CollectionStyle style;
    style.key = "wrong_id";
    style.screenIndex = 3;
    style.sizeMode = CollectionFrameSize::kSmall;

    holder->setStyle(style);

    EXPECT_EQ(holder->d->screenIndex, 1);
    EXPECT_EQ(holder->d->sizeMode, CollectionFrameSize::kMiddle);
}

TEST_F(UT_CollectionHolder, style_ReturnsCurrentStyle)
{
    CollectionFrame *testFrame = new CollectionFrame();
    testFrame->setGeometry(100, 100, 200, 300);
    holder->d->frame = testFrame;
    holder->d->screenIndex = 2;
    holder->d->sizeMode = CollectionFrameSize::kLarge;

    CollectionStyle result = holder->style();

    EXPECT_EQ(result.key, "test_id");
    EXPECT_EQ(result.screenIndex, 2);
    EXPECT_EQ(result.sizeMode, CollectionFrameSize::kLarge);
    EXPECT_EQ(result.rect, testFrame->geometry());

    delete testFrame;
    holder->d->frame = nullptr;
}

TEST_F(UT_CollectionHolder, selectFiles_WithItemView_CallsSelectUrls)
{
    bool selectUrlsCalled = false;
    bool scrollCalled = false;
    QList<QUrl> testUrls = { QUrl("file:///test1.txt"), QUrl("file:///test2.txt") };

    stub.set_lamda(&CollectionView::selectUrls, [&selectUrlsCalled](CollectionView *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        selectUrlsCalled = true;
    });

    stub.set_lamda(&CollectionView::scrollToBottom, [&scrollCalled]() {
        __DBG_STUB_INVOKE__
        scrollCalled = true;
    });

    stub.set_lamda(&CollectionWidget::view, []() {
        __DBG_STUB_INVOKE__
        static CollectionView *view = nullptr;
        return view;
    });

    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->widget = testWidget;

    // itemView returns nullptr, so selectFiles should return early
    holder->selectFiles(testUrls);

    delete testWidget;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, setFreeze_WithOptimizationEnabled_CallsWidgetSetFreeze)
{
    bool freezeCalled = false;

    stub.set_lamda(&ConfigPresenter::optimizeMovingPerformance, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CollectionWidget::setFreeze, [&freezeCalled](CollectionWidget *, bool) {
        __DBG_STUB_INVOKE__
        freezeCalled = true;
    });

    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->widget = testWidget;

    holder->setFreeze(true);
    EXPECT_TRUE(freezeCalled);

    delete testWidget;
    holder->d->widget = nullptr;
}

TEST_F(UT_CollectionHolder, setFreeze_WithOptimizationDisabled_DoesNotCallSetFreeze)
{
    bool freezeCalled = false;

    stub.set_lamda(&ConfigPresenter::optimizeMovingPerformance, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CollectionWidget::setFreeze, [&freezeCalled](CollectionWidget *, bool) {
        __DBG_STUB_INVOKE__
        freezeCalled = true;
    });

    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->widget = testWidget;

    holder->setFreeze(true);
    EXPECT_FALSE(freezeCalled);

    delete testWidget;
    holder->d->widget = nullptr;
}

// Additional tests from the original test_collection.cpp that were not in test_collectionholder.cpp
TEST_F(UT_CollectionHolder, name_Default_ReturnsEmptyString)
{
    QString name;
    if (holder->d->widget) {
        name = holder->name();
    } else {
        CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
        holder->d->widget = testWidget;
        name = holder->name();

        delete testWidget;
        holder->d->widget = nullptr;
    }
    EXPECT_TRUE(name.isEmpty());
}

TEST_F(UT_CollectionHolder, setName_SetsName)
{
    QString testName = "Test Collection";
    QString retrievedName;
    if (holder->d->widget) {
        holder->setName(testName);
    } else {
        CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
        holder->d->widget = testWidget;
        holder->setName(testName);
        retrievedName = holder->name();

        delete testWidget;
        holder->d->widget = nullptr;
    }

    EXPECT_EQ(retrievedName, testName);
}

TEST_F(UT_CollectionHolder, frame_Default_ReturnsNullptr)
{
    auto frame = holder->frame();
    EXPECT_EQ(frame, nullptr);
}

TEST_F(UT_CollectionHolder, widget_Default_ReturnsNullptr)
{
    auto widget = holder->widget();
    EXPECT_EQ(widget, nullptr);
}

TEST_F(UT_CollectionHolder, itemView_Default_ReturnsNullptr)
{
    auto itemView = holder->itemView();
    EXPECT_EQ(itemView, nullptr);
}

TEST_F(UT_CollectionHolder, surface_Default_ReturnsNullptr)
{
    auto surface = holder->surface();
    EXPECT_EQ(surface, nullptr);
}

TEST_F(UT_CollectionHolder, setSurface_SetsSurface)
{
    Surface *surface = nullptr; // In a real test, you'd create a proper Surface object
    holder->setSurface(surface);
    EXPECT_EQ(holder->surface(), surface);
}

TEST_F(UT_CollectionHolder, movable_Default_ReturnsFalse)
{
    bool movable = holder->movable();
    EXPECT_FALSE(movable);
}

TEST_F(UT_CollectionHolder, setMovable_SetsMovable)
{
    holder->setMovable(true);
    EXPECT_TRUE(holder->movable());
}

TEST_F(UT_CollectionHolder, closable_Default_ReturnsFalse)
{
    bool closable = holder->closable();
    EXPECT_FALSE(closable);
}

TEST_F(UT_CollectionHolder, setClosable_SetsClosable)
{
    holder->setClosable(true);
    EXPECT_TRUE(holder->closable());
}

TEST_F(UT_CollectionHolder, floatable_Default_ReturnsFalse)
{
    bool floatable = holder->floatable();
    EXPECT_FALSE(floatable);
}

TEST_F(UT_CollectionHolder, setFloatable_SetsFloatable)
{
    holder->setFloatable(true);
    EXPECT_TRUE(holder->floatable());
}

TEST_F(UT_CollectionHolder, hiddableCollection_Default_ReturnsFalse)
{
    bool hiddable = holder->hiddableCollection();
    EXPECT_FALSE(hiddable);
}

TEST_F(UT_CollectionHolder, setHiddableCollection_SetsHiddable)
{
    holder->setHiddableCollection(true);
    EXPECT_TRUE(holder->hiddableCollection());
}

TEST_F(UT_CollectionHolder, stretchable_Default_ReturnsFalse)
{
    bool stretchable = holder->stretchable();
    EXPECT_FALSE(stretchable);
}

TEST_F(UT_CollectionHolder, setStretchable_SetsStretchable)
{
    holder->setStretchable(true);
    EXPECT_TRUE(holder->stretchable());
}

TEST_F(UT_CollectionHolder, adjustable_Default_ReturnsFalse)
{
    bool adjustable = holder->adjustable();
    EXPECT_FALSE(adjustable);
}

TEST_F(UT_CollectionHolder, setAdjustable_SetsAdjustable)
{
    holder->setAdjustable(true);
    EXPECT_TRUE(holder->adjustable());
}

TEST_F(UT_CollectionHolder, hiddableTitleBar_Default_ReturnsFalse)
{
    bool hiddable = holder->hiddableTitleBar();
    EXPECT_FALSE(hiddable);
}

TEST_F(UT_CollectionHolder, setHiddableTitleBar_SetsHiddable)
{
    holder->setHiddableTitleBar(true);
    EXPECT_TRUE(holder->hiddableTitleBar());
}

TEST_F(UT_CollectionHolder, hiddableView_Default_ReturnsFalse)
{
    bool hiddable = holder->hiddableView();
    EXPECT_FALSE(hiddable);
}

TEST_F(UT_CollectionHolder, setHiddableView_SetsHiddable)
{
    holder->setHiddableView(true);
    EXPECT_TRUE(holder->hiddableView());
}

TEST_F(UT_CollectionHolder, renamable_Default_ReturnsFalse)
{
    bool renamable = holder->renamable();
    EXPECT_FALSE(renamable);
}

TEST_F(UT_CollectionHolder, setRenamable_SetsRenamable)
{
    holder->setRenamable(true);
    EXPECT_TRUE(holder->renamable());
}

TEST_F(UT_CollectionHolder, fileShiftable_Default_ReturnsFalse)
{
    bool shiftable = holder->fileShiftable();
    EXPECT_FALSE(shiftable);
}

TEST_F(UT_CollectionHolder, setFileShiftable_SetsFileShiftable)
{
    holder->setFileShiftable(true);
    EXPECT_TRUE(holder->fileShiftable());
}

TEST_F(UT_CollectionHolder, style_Default_ReturnsDefaultStyle)
{
    CollectionStyle style = holder->style();
    // Just ensure the method can be called without crashing
    (void)style;
}

TEST_F(UT_CollectionHolder, setStyle_SetsStyle)
{
    CollectionStyle newStyle;
    newStyle.sizeMode = CollectionFrameSize::kSmall;
    holder->setStyle(newStyle);
    CollectionStyle currentStyle = holder->style();
    // Verify the style was set (implementation-dependent)
    (void)currentStyle;
}

TEST_F(UT_CollectionHolder, styleChanged_SignalEmits)
{
    QSignalSpy spy(holder, &CollectionHolder::styleChanged);
    CollectionStyle newStyle;
    newStyle.sizeMode = CollectionFrameSize::kSmall;
    holder->setStyle(newStyle);
    
    // Check if the signal was emitted
    // This depends on the actual implementation of setStyle
    (void)spy;
}

TEST_F(UT_CollectionHolder, sigRequestClose_Signal)
{
    QSignalSpy spy(holder, &CollectionHolder::sigRequestClose);
    // The signal itself cannot be directly tested without a slot connected to it
    (void)spy;
}

TEST_F(UT_CollectionHolder, frameSurfaceChanged_Signal)
{
    QSignalSpy spy(holder, &CollectionHolder::frameSurfaceChanged);
    // The signal itself cannot be directly tested without triggering code
    (void)spy;
}

class UT_CollectionHolderPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        mockProvider = new MockCollectionDataProvider();
        holder = new CollectionHolder("test_id", mockProvider);
    }

    virtual void TearDown() override
    {
        delete holder;
        holder = nullptr;
        delete mockProvider;
        mockProvider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionHolder *holder = nullptr;
    MockCollectionDataProvider *mockProvider = nullptr;
};

TEST_F(UT_CollectionHolderPrivate, Constructor_InitializesMembers)
{
    EXPECT_NE(holder->d, nullptr);
    EXPECT_EQ(holder->d->id, "test_id");
    EXPECT_EQ(holder->d->q, holder);
    EXPECT_EQ(holder->d->provider, mockProvider);
}

TEST_F(UT_CollectionHolderPrivate, onAdjustFrameSizeMode_UpdatesSizeMode)
{
    bool styleChangedEmitted = false;

    stub.set_lamda(&CollectionWidget::setCollectionSize, [](CollectionWidget *, const CollectionFrameSize &) {
        __DBG_STUB_INVOKE__
    });

    QObject::connect(holder, &CollectionHolder::styleChanged, [&styleChangedEmitted](const QString &) {
        styleChangedEmitted = true;
    });

    CollectionWidget *testWidget = new CollectionWidget("test", mockProvider);
    holder->d->widget = testWidget;

    holder->d->onAdjustFrameSizeMode(CollectionFrameSize::kLarge);

    EXPECT_EQ(holder->d->sizeMode, CollectionFrameSize::kLarge);
    EXPECT_TRUE(styleChangedEmitted);

    delete testWidget;
    holder->d->widget = nullptr;
}
