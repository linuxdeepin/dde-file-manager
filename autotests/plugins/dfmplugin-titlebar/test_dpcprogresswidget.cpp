// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dialogs/dpcwidget/dpcprogresswidget.h"

#include <DLabel>
#include <DWaterProgress>
#include <DFontSizeManager>

#include <gtest/gtest.h>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QSignalSpy>
#include <QVBoxLayout>
#include <QTest>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class DPCProgressWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub QDBusConnection
        stub.set_lamda(&QDBusConnection::systemBus, []() {
            __DBG_STUB_INVOKE__
            return QDBusConnection("stub_system_bus");
        });

        // Stub QDBusInterface constructor
        stub.set_lamda(static_cast<bool (QDBusInterface::*)() const>(&QDBusInterface::isValid), [](QDBusInterface *) {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Stub QDBusConnection::connect
        stub.set_lamda(static_cast<bool (QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *)>(&QDBusConnection::connect),
                       [](QDBusConnection *, const QString &, const QString &, const QString &, const QString &, QObject *, const char *) {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        typedef void (DFontSizeManager::*Bind)(QWidget *, DFontSizeManager::SizeType, int);
        stub.set_lamda(static_cast<Bind>(&DFontSizeManager::bind), [](DFontSizeManager *, QWidget *, DFontSizeManager::SizeType, int) {
            __DBG_STUB_INVOKE__
        });

        widget = new DPCProgressWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    DPCProgressWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(DPCProgressWidgetTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(DPCProgressWidgetTest, Constructor_InitializesUIComponents_AllWidgetsCreated)
{
    EXPECT_NE(widget->titleLabel, nullptr);
    EXPECT_NE(widget->msgLabel, nullptr);
    EXPECT_NE(widget->changeProgress, nullptr);
    EXPECT_NE(widget->progressTimer, nullptr);
}

TEST_F(DPCProgressWidgetTest, Constructor_InitializesDBusInterface_InterfaceCreated)
{
    EXPECT_NE(widget->accessControlInter, nullptr);
}

TEST_F(DPCProgressWidgetTest, InitUI_TitleLabel_ConfiguredCorrectly)
{
    EXPECT_TRUE(widget->titleLabel->testAttribute(Qt::WA_TransparentForMouseEvents));
    EXPECT_EQ(widget->titleLabel->alignment(), Qt::AlignCenter);
    EXPECT_FALSE(widget->titleLabel->text().isEmpty());
}

TEST_F(DPCProgressWidgetTest, InitUI_MsgLabel_ConfiguredCorrectly)
{
    EXPECT_TRUE(widget->msgLabel->wordWrap());
    EXPECT_EQ(widget->msgLabel->alignment(), Qt::AlignHCenter);
    EXPECT_EQ(widget->msgLabel->minimumHeight(), 50);
    EXPECT_FALSE(widget->msgLabel->text().isEmpty());
}

TEST_F(DPCProgressWidgetTest, InitUI_ChangeProgress_ConfiguredCorrectly)
{
    EXPECT_EQ(widget->changeProgress->size(), QSize(98, 98));
    EXPECT_EQ(widget->changeProgress->value(), 1);
}

TEST_F(DPCProgressWidgetTest, InitUI_ProgressTimer_ConfiguredCorrectly)
{
    EXPECT_EQ(widget->progressTimer->interval(), 1000);
    EXPECT_FALSE(widget->progressTimer->isActive());
}

TEST_F(DPCProgressWidgetTest, InitUI_Layout_CreatedCorrectly)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(widget->layout());
    EXPECT_NE(layout, nullptr);
    if (layout) {
        EXPECT_EQ(layout->contentsMargins(), QMargins(0, 0, 0, 0));
    }
}

TEST_F(DPCProgressWidgetTest, Start_Called_StartsTimerAndProgress)
{
    // Stub DWaterProgress::start
    bool progressStarted = false;
    stub.set_lamda(&DWaterProgress::start, [&progressStarted](DWaterProgress *) {
        __DBG_STUB_INVOKE__
        progressStarted = true;
    });

    widget->start();

    EXPECT_TRUE(widget->progressTimer->isActive());
    EXPECT_TRUE(progressStarted);
}

TEST_F(DPCProgressWidgetTest, Start_CalledMultipleTimes_TimerRestarts)
{
    widget->start();
    EXPECT_TRUE(widget->progressTimer->isActive());

    widget->start();
    EXPECT_TRUE(widget->progressTimer->isActive());
}

TEST_F(DPCProgressWidgetTest, ChangeProgressValue_ValueLessThan90_IncreasesBy5)
{
    widget->changeProgress->setValue(50);

    widget->changeProgressValue();

    EXPECT_EQ(widget->changeProgress->value(), 55);
}

TEST_F(DPCProgressWidgetTest, ChangeProgressValue_ValueLessThan90_TimerContinues)
{
    widget->changeProgress->setValue(50);
    widget->start();

    widget->changeProgressValue();

    EXPECT_TRUE(widget->progressTimer->isActive());
}

TEST_F(DPCProgressWidgetTest, ChangeProgressValue_ValueAt90_StopsTimer)
{
    widget->changeProgress->setValue(90);
    widget->start();

    widget->changeProgressValue();

    EXPECT_FALSE(widget->progressTimer->isActive());
}

TEST_F(DPCProgressWidgetTest, ChangeProgressValue_ValueAt93_StopsTimer)
{
    widget->changeProgress->setValue(93);
    widget->start();

    widget->changeProgressValue();

    EXPECT_FALSE(widget->progressTimer->isActive());
}

TEST_F(DPCProgressWidgetTest, ChangeProgressValue_ValueNear95_DoesNotExceed)
{
    widget->changeProgress->setValue(92);
    widget->start();

    widget->changeProgressValue();

    EXPECT_FALSE(widget->progressTimer->isActive());
    // Value should not be increased since 92 + 5 >= 95
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_NoError_StopsTimerAndCompletesProgress)
{
    widget->start();
    EXPECT_TRUE(widget->progressTimer->isActive());

    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    // Stub QTimer::singleShot to execute immediately
    stub.set_lamda(static_cast<void (*)(int, const QObject *, const char *)>(&QTimer::singleShot),
                   [](int, const QObject *receiver, const char *member) {
                       __DBG_STUB_INVOKE__
                       QMetaObject::invokeMethod(const_cast<QObject *>(receiver), member + 1);
                   });

    widget->onDiskPwdChanged(kNoError);

    EXPECT_FALSE(widget->progressTimer->isActive());
    EXPECT_EQ(widget->changeProgress->value(), 100);
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_NoError_TimerNotActive_DoesNotCrash)
{
    // Don't start the timer
    EXPECT_FALSE(widget->progressTimer->isActive());

    stub.set_lamda(static_cast<void (*)(int, const QObject *, const char *)>(&QTimer::singleShot),
                   [](int, const QObject *receiver, const char *member) {
                       __DBG_STUB_INVOKE__
                       QMetaObject::invokeMethod(const_cast<QObject *>(receiver), member + 1);
                   });

    widget->onDiskPwdChanged(kNoError);

    EXPECT_EQ(widget->changeProgress->value(), 100);
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_PasswordInconsistent_EmitsCompletedWithError)
{
    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    widget->onDiskPwdChanged(kPasswordInconsistent);

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_FALSE(spy.at(0).at(0).toBool());
        QString errorMsg = spy.at(0).at(1).toString();
        EXPECT_FALSE(errorMsg.isEmpty());
        // Should contain error message about password inconsistency
    }
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_AccessDiskFailed_EmitsCompletedWithError)
{
    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    widget->onDiskPwdChanged(kAccessDiskFailed);

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_FALSE(spy.at(0).at(0).toBool());
        QString errorMsg = spy.at(0).at(1).toString();
        EXPECT_FALSE(errorMsg.isEmpty());
        // Should contain error message about disk access
    }
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_DeviceLoadFailed_EmitsCompletedWithError)
{
    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    widget->onDiskPwdChanged(kDeviceLoadFailed);

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_FALSE(spy.at(0).at(0).toBool());
        QString errorMsg = spy.at(0).at(1).toString();
        EXPECT_FALSE(errorMsg.isEmpty());
        // Should contain initialization error message
    }
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_InitFailed_EmitsCompletedWithError)
{
    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    widget->onDiskPwdChanged(kInitFailed);

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_FALSE(spy.at(0).at(0).toBool());
        QString errorMsg = spy.at(0).at(1).toString();
        EXPECT_FALSE(errorMsg.isEmpty());
        // Should contain initialization error message
    }
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_UnknownError_EmitsCompletedWithoutMessage)
{
    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    // Use an error code not in the switch statement
    widget->onDiskPwdChanged(999);

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_FALSE(spy.at(0).at(0).toBool());
        EXPECT_TRUE(spy.at(0).at(1).toString().isEmpty());
    }
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_AuthenticationFailed_EmitsCompletedWithoutMessage)
{
    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    widget->onDiskPwdChanged(kAuthenticationFailed);

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_FALSE(spy.at(0).at(0).toBool());
        EXPECT_TRUE(spy.at(0).at(1).toString().isEmpty());
    }
}

TEST_F(DPCProgressWidgetTest, ProgressTimer_Timeout_CallsChangeProgressValue)
{
    widget->changeProgress->setValue(10);
    widget->start();

    // Wait for timer to fire
    QTest::qWait(1100);

    // Value should have increased
    EXPECT_GT(widget->changeProgress->value(), 10);
}

TEST_F(DPCProgressWidgetTest, ProgressTimer_StopsAtLimit_DoesNotExceed95)
{
    widget->changeProgress->setValue(85);
    widget->start();

    // Fire timer multiple times until it stops
    for (int i = 0; i < 5; ++i) {
        if (widget->progressTimer->isActive()) {
            QTest::qWait(1100);
        }
    }

    EXPECT_LT(widget->changeProgress->value(), 95);
    EXPECT_FALSE(widget->progressTimer->isActive());
}

TEST_F(DPCProgressWidgetTest, SignalConnection_TimerTimeout_ConnectedCorrectly)
{
    // Verify that timer timeout is connected
    bool connected = QObject::disconnect(widget->progressTimer, &QTimer::timeout,
                                         widget, &DPCProgressWidget::changeProgressValue);
    EXPECT_TRUE(connected);

    // Reconnect for cleanup
    QObject::connect(widget->progressTimer, &QTimer::timeout,
                     widget, &DPCProgressWidget::changeProgressValue);
}

TEST_F(DPCProgressWidgetTest, TitleLabel_Text_ContainsExpectedContent)
{
    QString titleText = widget->titleLabel->text();
    EXPECT_FALSE(titleText.isEmpty());
    // Should contain text about changing password
}

TEST_F(DPCProgressWidgetTest, MsgLabel_Text_ContainsExpectedContent)
{
    QString msgText = widget->msgLabel->text();
    EXPECT_FALSE(msgText.isEmpty());
    // Should contain warning about not closing window
}

TEST_F(DPCProgressWidgetTest, ChangeProgress_InitialValue_IsOne)
{
    EXPECT_EQ(widget->changeProgress->value(), 1);
}

TEST_F(DPCProgressWidgetTest, ChangeProgress_Size_IsFixed)
{
    EXPECT_EQ(widget->changeProgress->width(), 98);
    EXPECT_EQ(widget->changeProgress->height(), 98);
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_Sequence_NoError_CompletesSuccessfully)
{
    widget->start();
    EXPECT_TRUE(widget->progressTimer->isActive());

    // Simulate progress updates
    widget->changeProgressValue();
    EXPECT_GT(widget->changeProgress->value(), 1);

    stub.set_lamda(static_cast<void (*)(int, const QObject *, const char *)>(&QTimer::singleShot),
                   [](int, const QObject *receiver, const char *member) {
                       __DBG_STUB_INVOKE__
                       QMetaObject::invokeMethod(const_cast<QObject *>(receiver), member + 1);
                   });

    // Simulate successful completion
    widget->onDiskPwdChanged(kNoError);

    EXPECT_FALSE(widget->progressTimer->isActive());
    EXPECT_EQ(widget->changeProgress->value(), 100);
}

TEST_F(DPCProgressWidgetTest, OnDiskPwdChanged_Sequence_Error_StopsImmediately)
{
    QSignalSpy spy(widget, &DPCProgressWidget::sigCompleted);

    widget->start();
    widget->changeProgressValue();

    // Simulate error during progress
    widget->onDiskPwdChanged(kPasswordInconsistent);

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_FALSE(spy.at(0).at(0).toBool());
    }
}

TEST_F(DPCProgressWidgetTest, AccessControlInterface_Service_ConfiguredCorrectly)
{
    EXPECT_NE(widget->accessControlInter, nullptr);
    if (widget->accessControlInter) {
        EXPECT_EQ(widget->accessControlInter->service(), "org.deepin.Filemanager.AccessControlManager");
        EXPECT_EQ(widget->accessControlInter->path(), "/org/deepin/Filemanager/AccessControlManager");
        EXPECT_EQ(widget->accessControlInter->interface(), "org.deepin.Filemanager.AccessControlManager");
    }
}

TEST_F(DPCProgressWidgetTest, ProgressTimer_Interval_Is1000ms)
{
    EXPECT_EQ(widget->progressTimer->interval(), 1000);
}

TEST_F(DPCProgressWidgetTest, Layout_ContainsAllWidgets)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(widget->layout());
    EXPECT_NE(layout, nullptr);

    if (layout) {
        bool foundTitle = false;
        bool foundProgress = false;
        bool foundMsg = false;

        for (int i = 0; i < layout->count(); ++i) {
            QLayoutItem *item = layout->itemAt(i);
            if (item && item->widget()) {
                if (item->widget() == widget->titleLabel)
                    foundTitle = true;
                if (item->widget() == widget->changeProgress)
                    foundProgress = true;
                if (item->widget() == widget->msgLabel)
                    foundMsg = true;
            }
        }

        EXPECT_TRUE(foundTitle);
        EXPECT_TRUE(foundProgress);
        EXPECT_TRUE(foundMsg);
    }
}

TEST_F(DPCProgressWidgetTest, TitleLabel_SizePolicy_IsExpanding)
{
    EXPECT_EQ(widget->titleLabel->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    EXPECT_EQ(widget->titleLabel->sizePolicy().verticalPolicy(), QSizePolicy::Expanding);
}

TEST_F(DPCProgressWidgetTest, ChangeProgressValue_EdgeCase_Value94_StopsTimer)
{
    widget->changeProgress->setValue(94);
    widget->start();

    widget->changeProgressValue();

    EXPECT_FALSE(widget->progressTimer->isActive());
}

TEST_F(DPCProgressWidgetTest, ChangeProgressValue_EdgeCase_Value1_IncreasesTo6)
{
    widget->changeProgress->setValue(1);

    widget->changeProgressValue();

    EXPECT_EQ(widget->changeProgress->value(), 6);
}
