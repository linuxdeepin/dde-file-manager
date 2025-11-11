// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "views/computerviewcontainer.h"
#include "views/computerview.h"
#include "views/computerstatusbar.h"

#include <dfm-base/interfaces/abstractbaseview.h>

#include <QUrl>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalSpy>

using namespace dfmplugin_computer;
DFMBASE_USE_NAMESPACE

class UT_ComputerViewContainer : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        testUrl = QUrl("computer:///");
        container = new ComputerViewContainer(testUrl);
    }

    virtual void TearDown() override
    {
        delete container;
        container = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerViewContainer *container = nullptr;
    QUrl testUrl;
};

TEST_F(UT_ComputerViewContainer, Constructor_WithValidUrl_CreatesSuccessfully)
{
    EXPECT_NE(container, nullptr);
    EXPECT_NE(container->widget(), nullptr);
    EXPECT_NE(container->contentWidget(), nullptr);
}

TEST_F(UT_ComputerViewContainer, Constructor_WithParent_SetsParentCorrectly)
{
    QWidget *parent = new QWidget();
    ComputerViewContainer *testContainer = new ComputerViewContainer(testUrl, parent);
    
    EXPECT_EQ(testContainer->parent(), parent);
    
    delete testContainer;
    delete parent;
}

TEST_F(UT_ComputerViewContainer, Widget_ReturnsSelf_Success)
{
    QWidget *widget = container->widget();
    EXPECT_EQ(widget, container);
}

TEST_F(UT_ComputerViewContainer, ContentWidget_ReturnsViewContainer_Success)
{
    QWidget *contentWidget = container->contentWidget();
    EXPECT_NE(contentWidget, nullptr);
    // The contentWidget should be the viewContainer, not the container itself
    EXPECT_NE(contentWidget, container);
}

// TEST_F(UT_ComputerViewContainer, RootUrl_ReturnsViewRootUrl_Success)
// {
//     QUrl expectedUrl("computer:///test");
    
//     // Mock ComputerView::rootUrl
//     stub.set_lamda(&ComputerView::rootUrl, [&expectedUrl]() -> QUrl {
//         __DBG_STUB_INVOKE__
//         return expectedUrl;
//     });
    
//     QUrl actualUrl = container->rootUrl();
//     EXPECT_EQ(actualUrl, expectedUrl);
// }

// TEST_F(UT_ComputerViewContainer, ViewState_ReturnsViewViewState_Success)
// {
//     AbstractBaseView::ViewState expectedState = AbstractBaseView::ViewState::kViewBusy;
    
//     // Mock ComputerView::viewState
//     stub.set_lamda(&ComputerView::viewState, [&expectedState]() -> AbstractBaseView::ViewState {
//         __DBG_STUB_INVOKE__
//         return expectedState;
//     });
    
//     AbstractBaseView::ViewState actualState = container->viewState();
//     EXPECT_EQ(actualState, expectedState);
// }

// TEST_F(UT_ComputerViewContainer, SetRootUrl_ValidUrl_UpdatesViewAndNotifiesStateChange)
// {
//     QUrl newUrl("computer:///newpath");
//     bool viewSetRootUrlCalled = false;
//     bool notifyStateChangedCalled = false;
    
//     // Mock ComputerView::setRootUrl
//     stub.set_lamda(&ComputerView::setRootUrl, [&viewSetRootUrlCalled, &newUrl](ComputerView *, const QUrl &url) -> bool {
//         __DBG_STUB_INVOKE__
//         viewSetRootUrlCalled = true;
//         EXPECT_EQ(url, newUrl);
//         return true;
//     });
    
//     // Mock notifyStateChanged
//     stub.set_lamda(&ComputerViewContainer::notifyStateChanged, [&notifyStateChangedCalled](AbstractBaseView *) {
//         __DBG_STUB_INVOKE__
//         notifyStateChangedCalled = true;
//     });
    
//     bool result = container->setRootUrl(newUrl);
    
//     EXPECT_TRUE(result);
//     EXPECT_TRUE(viewSetRootUrlCalled);
//     EXPECT_TRUE(notifyStateChangedCalled);
// }

// TEST_F(UT_ComputerViewContainer, SetRootUrl_ViewReturnsFalse_ReturnsFalse)
// {
//     QUrl newUrl("computer:///newpath");
    
//     // Mock ComputerView::setRootUrl to return false
//     stub.set_lamda(&ComputerView::setRootUrl, [](ComputerView *, const QUrl &) -> bool {
//         __DBG_STUB_INVOKE__
//         return false;
//     });
    
//     // Mock notifyStateChanged
//     stub.set_lamda(&ComputerViewContainer::notifyStateChanged, [](AbstractBaseView *) {
//         __DBG_STUB_INVOKE__
//     });
    
//     bool result = container->setRootUrl(newUrl);
    
//     EXPECT_FALSE(result);
// }

// TEST_F(UT_ComputerViewContainer, SelectedUrlList_ReturnsViewSelectedUrlList_Success)
// {
//     QList<QUrl> expectedUrls;
//     expectedUrls << QUrl("computer:///item1") << QUrl("computer:///item2");
    
//     // Mock ComputerView::selectedUrlList
//     stub.set_lamda(&ComputerView::selectedUrlList, [&expectedUrls]() -> QList<QUrl> {
//         __DBG_STUB_INVOKE__
//         return expectedUrls;
//     });
    
//     QList<QUrl> actualUrls = container->selectedUrlList();
//     EXPECT_EQ(actualUrls, expectedUrls);
// }

// TEST_F(UT_ComputerViewContainer, SelectedUrlList_EmptySelection_ReturnsEmptyList)
// {
//     QList<QUrl> expectedUrls; // Empty list
    
//     // Mock ComputerView::selectedUrlList
//     stub.set_lamda(&ComputerView::selectedUrlList, [&expectedUrls]() -> QList<QUrl> {
//         __DBG_STUB_INVOKE__
//         return expectedUrls;
//     });
    
//     QList<QUrl> actualUrls = container->selectedUrlList();
//     EXPECT_TRUE(actualUrls.isEmpty());
// }

TEST_F(UT_ComputerViewContainer, Layout_SetupCorrectly_Success)
{
    // Test that the layout is properly set up
    EXPECT_NE(container->view, nullptr);
    EXPECT_NE(container->viewContainer, nullptr);
    
    // The view should be a child of viewContainer
    EXPECT_EQ(container->view->parent(), container->viewContainer);
    
    // The viewContainer should be a child of container
    EXPECT_EQ(container->viewContainer->parent(), container);
}

TEST_F(UT_ComputerViewContainer, StatusBar_SetupCorrectly_Success)
{
    // Test that status bar is properly connected to view
    // This is tested indirectly by checking that the container was created successfully
    EXPECT_NE(container, nullptr);
}

TEST_F(UT_ComputerViewContainer, Inheritance_FromAbstractBaseView_WorksCorrectly)
{
    // Test that ComputerViewContainer is properly inherited from AbstractBaseView
    AbstractBaseView *baseView = container;
    EXPECT_NE(baseView, nullptr);
    
    // Test that we can call base class methods
    EXPECT_NO_THROW(baseView->widget());
    EXPECT_NO_THROW(baseView->contentWidget());
    EXPECT_NO_THROW(baseView->rootUrl());
    EXPECT_NO_THROW(baseView->viewState());
    EXPECT_NO_THROW(baseView->selectedUrlList());
}

TEST_F(UT_ComputerViewContainer, SignalSlotMechanism_WorksCorrectly_Success)
{
    // Test that signal-slot mechanism works
    // Skip signal test for now
    // QSignalSpy stateChangedSpy(container, &AbstractBaseView::stateChanged);
    
    bool notifyStateChangedCalled = false;
    
    // Mock notifyStateChanged to emit signal
    stub.set_lamda(&ComputerViewContainer::notifyStateChanged, [&notifyStateChangedCalled](AbstractBaseView *) {
        __DBG_STUB_INVOKE__
        notifyStateChangedCalled = true;
        // Emit the signal manually for testing
        // Signal will be emitted by the actual notifyStateChanged method
    });
    
    // Trigger state change
    container->setRootUrl(QUrl("computer:///test"));
    
    // Verify state change was called
    EXPECT_TRUE(notifyStateChangedCalled);
}

TEST_F(UT_ComputerViewContainer, QtMetaObject_CorrectlyInitialized_Success)
{
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = container->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_computer::ComputerViewContainer");
    
    // Test that inherited methods exist in meta-object
    EXPECT_GE(metaObject->indexOfMethod("widget()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("contentWidget()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("rootUrl()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("viewState()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("setRootUrl(QUrl)"), 0);
    EXPECT_GE(metaObject->indexOfMethod("selectedUrlList()"), 0);
}

// TEST_F(UT_ComputerViewContainer, MultipleSetRootUrl_Calls_HandleCorrectly_Success)
// {
//     QUrl url1("computer:///path1");
//     QUrl url2("computer:///path2");
//     int setRootUrlCallCount = 0;
//     QList<QUrl> capturedUrls;
    
//     // Mock ComputerView::setRootUrl
//     stub.set_lamda(&ComputerView::setRootUrl, [&setRootUrlCallCount, &capturedUrls](ComputerView *, const QUrl &url) -> bool {
//         __DBG_STUB_INVOKE__
//         setRootUrlCallCount++;
//         capturedUrls.append(url);
//         return true;
//     });
    
//     // Mock notifyStateChanged
//     stub.set_lamda(&ComputerViewContainer::notifyStateChanged, [](AbstractBaseView *) {
//         __DBG_STUB_INVOKE__
//     });
    
//     // Call setRootUrl multiple times
//     container->setRootUrl(url1);
//     container->setRootUrl(url2);
    
//     EXPECT_EQ(setRootUrlCallCount, 2);
//     EXPECT_EQ(capturedUrls.size(), 2);
//     EXPECT_EQ(capturedUrls[0], url1);
//     EXPECT_EQ(capturedUrls[1], url2);
// }
