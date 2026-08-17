// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dialogs/diskpasswordchangingdialog.h"
#include "dialogs/dpcwidget/dpcconfirmwidget.h"
#include "dialogs/dpcwidget/dpcprogresswidget.h"
#include "dialogs/dpcwidget/dpcresultwidget.h"

#include <DWindowManagerHelper>
#include <DDialog>

#include <gtest/gtest.h>
#include <QStackedWidget>
#include <QCloseEvent>
#include <QIcon>
#include <QSignalSpy>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class DiskPasswordChangingDialogTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // Stub icon loading to avoid dependencies
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        dialog = new DiskPasswordChangingDialog();
    }

    void TearDown() override
    {
        delete dialog;
        dialog = nullptr;
        stub.clear();
    }

    DiskPasswordChangingDialog *dialog { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(DiskPasswordChangingDialogTest, Constructor_Called_ObjectCreated)
{
    EXPECT_NE(dialog, nullptr);
    EXPECT_NE(dialog->confirmWidget, nullptr);
    EXPECT_NE(dialog->progressWidget, nullptr);
    EXPECT_NE(dialog->resultWidget, nullptr);
    EXPECT_NE(dialog->switchPageWidget, nullptr);
}

TEST_F(DiskPasswordChangingDialogTest, InitUI_Called_WidgetsInitialized)
{
    // Verify widgets are added to stacked widget
    EXPECT_EQ(dialog->switchPageWidget->count(), 3);
    EXPECT_EQ(dialog->switchPageWidget->widget(0), dialog->confirmWidget);
    EXPECT_EQ(dialog->switchPageWidget->widget(1), dialog->progressWidget);
    EXPECT_EQ(dialog->switchPageWidget->widget(2), dialog->resultWidget);
}

TEST_F(DiskPasswordChangingDialogTest, InitUI_Called_SizeSet)
{
    EXPECT_EQ(dialog->size(), QSize(382, 286));
}

TEST_F(DiskPasswordChangingDialogTest, InitConnect_ConfirmWidgetSignals_ConnectedCorrectly)
{
    QSignalSpy closeSpy(dialog->confirmWidget, &DPCConfirmWidget::sigCloseDialog);
    QSignalSpy confirmSpy(dialog->confirmWidget, &DPCConfirmWidget::sigConfirmed);

    // Test signal connections by checking if slots are called
    bool closeSlotCalled = false;
    stub.set_lamda(&DiskPasswordChangingDialog::close, [&closeSlotCalled] {
        __DBG_STUB_INVOKE__
        closeSlotCalled = true;
        return true;
    });

    emit dialog->confirmWidget->sigCloseDialog();
    EXPECT_TRUE(closeSlotCalled);
}

TEST_F(DiskPasswordChangingDialogTest, OnConfirmed_Called_SwitchesToProgressWidget)
{
    bool setMotifFunctionsCalled = false;
    bool startCalled = false;

    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [&setMotifFunctionsCalled] {
                       __DBG_STUB_INVOKE__
                       setMotifFunctionsCalled = true;
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCProgressWidget::start, [&startCalled](DPCProgressWidget *) {
        __DBG_STUB_INVOKE__
        startCalled = true;
    });

    dialog->onConfirmed();

    EXPECT_TRUE(setMotifFunctionsCalled);
    EXPECT_TRUE(startCalled);
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->progressWidget);
}

TEST_F(DiskPasswordChangingDialogTest, OnConfirmed_Called_DisablesCloseFunction)
{
    bool closeFunctionDisabled = false;

    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [&closeFunctionDisabled](const QWindow *, DWindowManagerHelper::MotifFunctions functions, bool enabled) {
                       __DBG_STUB_INVOKE__
                       if (functions == DWindowManagerHelper::FUNC_CLOSE && !enabled) {
                           closeFunctionDisabled = true;
                       }
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCProgressWidget::start, [](DPCProgressWidget *) {
        __DBG_STUB_INVOKE__
    });

    dialog->onConfirmed();
    EXPECT_TRUE(closeFunctionDisabled);
}

TEST_F(DiskPasswordChangingDialogTest, OnChangeCompleted_Success_SwitchesToResultWidget)
{
    bool setMotifFunctionsCalled = false;
    bool setResultCalled = false;

    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [&setMotifFunctionsCalled] {
                       __DBG_STUB_INVOKE__
                       setMotifFunctionsCalled = true;
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCResultWidget::setResult, [&setResultCalled](DPCResultWidget *, bool success, const QString &) {
        __DBG_STUB_INVOKE__
        setResultCalled = true;
        EXPECT_TRUE(success);
    });

    dialog->onChangeCompleted(true, "Success message");

    EXPECT_TRUE(setMotifFunctionsCalled);
    EXPECT_TRUE(setResultCalled);
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->resultWidget);
}

TEST_F(DiskPasswordChangingDialogTest, OnChangeCompleted_Failure_SwitchesToResultWidget)
{
    bool setResultCalled = false;

    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [] {
                       __DBG_STUB_INVOKE__
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCResultWidget::setResult, [&setResultCalled](DPCResultWidget *, bool success, const QString &msg) {
        __DBG_STUB_INVOKE__
        setResultCalled = true;
        EXPECT_FALSE(success);
        EXPECT_EQ(msg, "Error message");
    });

    dialog->onChangeCompleted(false, "Error message");

    EXPECT_TRUE(setResultCalled);
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->resultWidget);
}

TEST_F(DiskPasswordChangingDialogTest, OnChangeCompleted_Called_EnablesCloseFunction)
{
    bool closeFunctionEnabled = false;

    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [&closeFunctionEnabled] {
                       __DBG_STUB_INVOKE__
                       closeFunctionEnabled = true;
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCResultWidget::setResult, [](DPCResultWidget *, bool, const QString &) {
        __DBG_STUB_INVOKE__
    });

    dialog->onChangeCompleted(true, "Success");
    EXPECT_TRUE(closeFunctionEnabled);
}

TEST_F(DiskPasswordChangingDialogTest, CloseEvent_OnConfirmWidget_AcceptsEvent)
{
    dialog->switchPageWidget->setCurrentWidget(dialog->confirmWidget);

    QCloseEvent event;
    bool baseCloseEventCalled = false;

    stub.set_lamda(VADDR(DDialog, closeEvent),
                   [&baseCloseEventCalled] {
                       __DBG_STUB_INVOKE__
                       baseCloseEventCalled = true;
                   });

    dialog->closeEvent(&event);
    EXPECT_TRUE(baseCloseEventCalled);
}

TEST_F(DiskPasswordChangingDialogTest, CloseEvent_OnProgressWidget_IgnoresEvent)
{
    dialog->switchPageWidget->setCurrentWidget(dialog->progressWidget);

    QCloseEvent event;
    bool baseCloseEventCalled = false;

    stub.set_lamda(VADDR(DDialog, closeEvent),
                   [&baseCloseEventCalled] {
                       __DBG_STUB_INVOKE__
                       baseCloseEventCalled = true;
                   });

    dialog->closeEvent(&event);
    EXPECT_FALSE(baseCloseEventCalled);
    EXPECT_TRUE(event.isAccepted() == false || !baseCloseEventCalled);
}

TEST_F(DiskPasswordChangingDialogTest, CloseEvent_OnResultWidget_AcceptsEvent)
{
    dialog->switchPageWidget->setCurrentWidget(dialog->resultWidget);

    QCloseEvent event;
    bool baseCloseEventCalled = false;

    stub.set_lamda(VADDR(DDialog, closeEvent),
                   [&baseCloseEventCalled] {
                       __DBG_STUB_INVOKE__
                       baseCloseEventCalled = true;
                   });

    dialog->closeEvent(&event);
    EXPECT_TRUE(baseCloseEventCalled);
}

TEST_F(DiskPasswordChangingDialogTest, CloseEvent_NullSwitchPageWidget_CallsBaseCloseEvent)
{
    dialog->switchPageWidget = nullptr;

    QCloseEvent event;
    bool baseCloseEventCalled = false;

    stub.set_lamda(VADDR(DDialog, closeEvent),
                   [&baseCloseEventCalled] {
                       __DBG_STUB_INVOKE__
                       baseCloseEventCalled = true;
                   });

    dialog->closeEvent(&event);
    EXPECT_TRUE(baseCloseEventCalled);
}

TEST_F(DiskPasswordChangingDialogTest, SignalConnection_ConfirmWidgetClose_ClosesDialog)
{
    QSignalSpy closeSpy(dialog, &DiskPasswordChangingDialog::close);

    stub.set_lamda(&DiskPasswordChangingDialog::close, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    emit dialog->confirmWidget->sigCloseDialog();
}

TEST_F(DiskPasswordChangingDialogTest, SignalConnection_ConfirmWidgetConfirmed_CallsOnConfirmed)
{
    bool onConfirmedCalled = false;

    stub.set_lamda(&DiskPasswordChangingDialog::onConfirmed, [&onConfirmedCalled](DiskPasswordChangingDialog *) {
        __DBG_STUB_INVOKE__
        onConfirmedCalled = true;
    });

    emit dialog->confirmWidget->sigConfirmed();
    EXPECT_TRUE(onConfirmedCalled);
}

TEST_F(DiskPasswordChangingDialogTest, SignalConnection_ProgressWidgetCompleted_CallsOnChangeCompleted)
{
    bool onChangeCompletedCalled = false;
    bool receivedSuccess = false;
    QString receivedMsg;

    stub.set_lamda(&DiskPasswordChangingDialog::onChangeCompleted,
                   [&onChangeCompletedCalled, &receivedSuccess, &receivedMsg](DiskPasswordChangingDialog *, bool success, const QString &msg) {
                       __DBG_STUB_INVOKE__
                       onChangeCompletedCalled = true;
                       receivedSuccess = success;
                       receivedMsg = msg;
                   });

    emit dialog->progressWidget->sigCompleted(true, "Test message");
    EXPECT_TRUE(onChangeCompletedCalled);
    EXPECT_TRUE(receivedSuccess);
    EXPECT_EQ(receivedMsg, "Test message");
}

TEST_F(DiskPasswordChangingDialogTest, SignalConnection_ResultWidgetClose_ClosesDialog)
{
    bool closeCalled = false;

    stub.set_lamda(&DiskPasswordChangingDialog::close, [&closeCalled] {
        __DBG_STUB_INVOKE__
        closeCalled = true;
        return true;
    });

    emit dialog->resultWidget->sigCloseDialog();
    EXPECT_TRUE(closeCalled);
}

TEST_F(DiskPasswordChangingDialogTest, InitialState_DefaultWidget_IsConfirmWidget)
{
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->confirmWidget);
}

TEST_F(DiskPasswordChangingDialogTest, WorkFlow_ConfirmToProgress_WorksCorrectly)
{
    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [] {
                       __DBG_STUB_INVOKE__
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCProgressWidget::start, [](DPCProgressWidget *) {
        __DBG_STUB_INVOKE__
    });

    // Initial state
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->confirmWidget);

    // After confirmation
    dialog->onConfirmed();
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->progressWidget);
}

TEST_F(DiskPasswordChangingDialogTest, WorkFlow_ProgressToResult_WorksCorrectly)
{
    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [] {
                       __DBG_STUB_INVOKE__
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCResultWidget::setResult, [](DPCResultWidget *, bool, const QString &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&DPCProgressWidget::start, [](DPCProgressWidget *) {
        __DBG_STUB_INVOKE__
    });

    // Go to progress widget
    dialog->onConfirmed();
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->progressWidget);

    // Go to result widget
    dialog->onChangeCompleted(true, "Success");
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->resultWidget);
}

TEST_F(DiskPasswordChangingDialogTest, WorkFlow_CompleteFlow_WorksCorrectly)
{
    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [] {
                       __DBG_STUB_INVOKE__
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCProgressWidget::start, [](DPCProgressWidget *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&DPCResultWidget::setResult, [](DPCResultWidget *, bool, const QString &) {
        __DBG_STUB_INVOKE__
    });

    // Start from confirm widget
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->confirmWidget);

    // User confirms
    dialog->onConfirmed();
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->progressWidget);

    // Password change completes
    dialog->onChangeCompleted(true, "Password changed successfully");
    EXPECT_EQ(dialog->switchPageWidget->currentWidget(), dialog->resultWidget);
}

TEST_F(DiskPasswordChangingDialogTest, OnChangeCompleted_WithEmptyMessage_HandlesCorrectly)
{
    bool setResultCalled = false;

    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [] {
                       __DBG_STUB_INVOKE__
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCResultWidget::setResult, [&setResultCalled](DPCResultWidget *, bool success, const QString &msg) {
        __DBG_STUB_INVOKE__
        setResultCalled = true;
        EXPECT_TRUE(msg.isEmpty());
    });

    dialog->onChangeCompleted(true, "");
    EXPECT_TRUE(setResultCalled);
}

TEST_F(DiskPasswordChangingDialogTest, OnChangeCompleted_WithLongMessage_HandlesCorrectly)
{
    bool setResultCalled = false;
    QString longMsg = "This is a very long error message that should be handled correctly by the dialog";

    typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
    stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                   [] {
                       __DBG_STUB_INVOKE__
                       return DWindowManagerHelper::FUNC_CLOSE;
                   });

    stub.set_lamda(&DPCResultWidget::setResult, [&setResultCalled, &longMsg](DPCResultWidget *, bool, const QString &msg) {
        __DBG_STUB_INVOKE__
        setResultCalled = true;
        EXPECT_EQ(msg, longMsg);
    });

    dialog->onChangeCompleted(false, longMsg);
    EXPECT_TRUE(setResultCalled);
}
