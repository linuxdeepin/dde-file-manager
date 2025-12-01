// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/shred/progressdialog.h"

#include <DWaterProgress>
#include <DLabel>

#include <QKeyEvent>
#include <QStackedWidget>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DWIDGET_USE_NAMESPACE

// ========== ProgressWidget Tests ==========

class UT_ProgressWidget : public testing::Test
{
protected:
    void SetUp() override
    {
        widget = new ProgressWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    ProgressWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ProgressWidget, Constructor_InitializesWidget)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(UT_ProgressWidget, setValue_UpdatesProgress)
{
    bool startCalled = false;

    stub.set_lamda(ADDR(DWaterProgress, start),
                   [&startCalled](DWaterProgress *) {
                       __DBG_STUB_INVOKE__
                       startCalled = true;
                   });

    stub.set_lamda(ADDR(DWaterProgress, value),
                   [](DWaterProgress *) -> int {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    stub.set_lamda(ADDR(DWaterProgress, setValue),
                   [](DWaterProgress *, int) {
                       __DBG_STUB_INVOKE__
                   });

    widget->setValue(50, "test.txt");

    EXPECT_TRUE(startCalled);
}

TEST_F(UT_ProgressWidget, setValue_At100_ShowsMessage)
{
    stub.set_lamda(ADDR(DWaterProgress, start),
                   [](DWaterProgress *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(ADDR(DWaterProgress, value),
                   [](DWaterProgress *) -> int {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    stub.set_lamda(ADDR(DWaterProgress, setValue),
                   [](DWaterProgress *, int) {
                       __DBG_STUB_INVOKE__
                   });

    widget->setValue(100, "Complete");
}

TEST_F(UT_ProgressWidget, stopProgress_StopsWaterProgress)
{
    bool stopCalled = false;

    stub.set_lamda(ADDR(DWaterProgress, stop),
                   [&stopCalled](DWaterProgress *) {
                       __DBG_STUB_INVOKE__
                       stopCalled = true;
                   });

    widget->stopProgress();

    EXPECT_TRUE(stopCalled);
}

// ========== ShredFailedWidget Tests ==========

class UT_ShredFailedWidget : public testing::Test
{
protected:
    void SetUp() override
    {
        widget = new ShredFailedWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    ShredFailedWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ShredFailedWidget, Constructor_InitializesWidget)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(UT_ShredFailedWidget, setMessage_ShortMessage_ShowsFull)
{
    widget->setMessage("Short error");
}

TEST_F(UT_ShredFailedWidget, setMessage_LongMessage_Truncates)
{
    QString longMessage = QString("A").repeated(100);
    widget->setMessage(longMessage);
}

// ========== ProgressDialog Tests ==========

class UT_ProgressDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        dialog = new ProgressDialog();
    }

    void TearDown() override
    {
        delete dialog;
        dialog = nullptr;
        stub.clear();
    }

    ProgressDialog *dialog { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ProgressDialog, Constructor_InitializesDialog)
{
    EXPECT_NE(dialog, nullptr);
}

TEST_F(UT_ProgressDialog, updateProgressValue_ValidValue_UpdatesProgress)
{
    stub.set_lamda(ADDR(DWaterProgress, start),
                   [](DWaterProgress *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(ADDR(DWaterProgress, value),
                   [](DWaterProgress *) -> int {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    stub.set_lamda(ADDR(DWaterProgress, setValue),
                   [](DWaterProgress *, int) {
                       __DBG_STUB_INVOKE__
                   });

    dialog->updateProgressValue(50, "test.txt");
}

TEST_F(UT_ProgressDialog, updateProgressValue_InvalidValue_Ignores)
{
    dialog->updateProgressValue(-1, "test.txt");
    dialog->updateProgressValue(150, "test.txt");
}

TEST_F(UT_ProgressDialog, handleShredResult_Failure_ShowsFailedWidget)
{
    stub.set_lamda(ADDR(DWaterProgress, stop),
                   [](DWaterProgress *) {
                       __DBG_STUB_INVOKE__
                   });

    dialog->handleShredResult(false, "Error message");
}

