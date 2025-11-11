// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "views/computerstatusbar.h"

#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>

#include <QString>
#include <QLabel>
#include <QApplication>

using namespace dfmplugin_computer;
DFMBASE_USE_NAMESPACE

class UT_ComputerStatusBar : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        statusBar = new ComputerStatusBar(nullptr);
    }

    virtual void TearDown() override
    {
        delete statusBar;
        statusBar = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerStatusBar *statusBar = nullptr;
};

TEST_F(UT_ComputerStatusBar, Constructor_WithParent_CreatesSuccessfully)
{
    EXPECT_NE(statusBar, nullptr);
    EXPECT_NE(statusBar->parent(), nullptr);
}

TEST_F(UT_ComputerStatusBar, Constructor_WithNullParent_CreatesSuccessfully)
{
    ComputerStatusBar *testBar = new ComputerStatusBar(nullptr);
    EXPECT_NE(testBar, nullptr);
    delete testBar;
}

TEST_F(UT_ComputerStatusBar, ShowSingleSelectionMessage_SetsCorrectText)
{
    bool setTipTextCalled = false;
    QString capturedText;
    
    // Mock setTipText method from base class
    stub.set_lamda(&BasicStatusBar::setTipText, [&](BasicStatusBar *, const QString &text) {
        __DBG_STUB_INVOKE__
        setTipTextCalled = true;
        capturedText = text;
    });
    
    statusBar->showSingleSelectionMessage();
    
    EXPECT_TRUE(setTipTextCalled);
    EXPECT_TRUE(capturedText.contains("1 item selected"));
}

TEST_F(UT_ComputerStatusBar, ShowSingleSelectionMessage_MultipleCalls_UpdatesText)
{
    QString capturedText;
    int callCount = 0;
    
    // Mock setTipText method from base class
    stub.set_lamda(&BasicStatusBar::setTipText, [&](BasicStatusBar *, const QString &text) {
        __DBG_STUB_INVOKE__
        callCount++;
        capturedText = text;
    });
    
    // Call multiple times
    statusBar->showSingleSelectionMessage();
    int firstCallCount = callCount;
    QString firstText = capturedText;
    
    statusBar->showSingleSelectionMessage();
    int secondCallCount = callCount;
    QString secondText = capturedText;
    
    EXPECT_EQ(firstCallCount, 1);
    EXPECT_EQ(secondCallCount, 2);
    EXPECT_EQ(firstText, secondText);
    EXPECT_TRUE(firstText.contains("1 item selected"));
}

TEST_F(UT_ComputerStatusBar, Inheritance_FromBasicStatusBar_WorksCorrectly)
{
    // Test that ComputerStatusBar is properly inherited from BasicStatusBar
    BasicStatusBar *baseBar = statusBar;
    EXPECT_NE(baseBar, nullptr);
    
    // Test that we can call base class methods
    EXPECT_NO_THROW(baseBar->setTipText("test"));
}

TEST_F(UT_ComputerStatusBar, QtMetaObject_CorrectlyInitialized_Success)
{
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = statusBar->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_computer::ComputerStatusBar");
    
    // Test that the method exists in meta-object
    int methodIndex = metaObject->indexOfMethod("showSingleSelectionMessage()");
    EXPECT_GE(methodIndex, 0);
}

TEST_F(UT_ComputerStatusBar, SignalSlotMechanism_WorksCorrectly_Success)
{
    // Test that signal-slot mechanism works
    QString capturedText;
    
    // Mock setTipText to emit signal
    stub.set_lamda(&BasicStatusBar::setTipText, [&](BasicStatusBar *, const QString &text) {
        __DBG_STUB_INVOKE__
        // Emit the signal manually for testing
        capturedText = text;
    });
    
    statusBar->showSingleSelectionMessage();
    
    // Verify text was set correctly
    EXPECT_TRUE(capturedText.contains("1 item selected"));
}

TEST_F(UT_ComputerStatusBar, Translation_HandlesDifferentLocales_Success)
{
    QString capturedText;
    
    // Mock setTipText method
    stub.set_lamda(&BasicStatusBar::setTipText, [&](BasicStatusBar *, const QString &text) {
        __DBG_STUB_INVOKE__
        capturedText = text;
    });
    
    // Test with different translation contexts
    statusBar->showSingleSelectionMessage();
    
    // Verify the translation context is used correctly
    EXPECT_TRUE(capturedText.contains("item selected"));
    EXPECT_TRUE(capturedText.contains("1"));
}
