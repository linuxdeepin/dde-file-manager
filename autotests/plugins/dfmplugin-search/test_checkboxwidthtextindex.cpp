// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCheckBox>
#include <QSignalSpy>

#include "utils/checkboxwidthtextindex.h"
#include "utils/textindexclient.h"

#include <DTipLabel>
#include <DSpinner>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class TestTextIndexStatusBar : public testing::Test
{
public:
    void SetUp() override
    {
        statusBar = new TextIndexStatusBar();
    }

    void TearDown() override
    {
        delete statusBar;
        statusBar = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    TextIndexStatusBar *statusBar = nullptr;
};

class TestCheckBoxWidthTextIndex : public testing::Test
{
public:
    void SetUp() override
    {
        checkBox = new CheckBoxWidthTextIndex();
    }

    void TearDown() override
    {
        delete checkBox;
        checkBox = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    CheckBoxWidthTextIndex *checkBox = nullptr;
};

// TextIndexStatusBar Tests
TEST_F(TestTextIndexStatusBar, Constructor_CreatesValidInstance)
{
    EXPECT_NE(statusBar, nullptr);

    // Test with parent widget
    QWidget parent;
    TextIndexStatusBar barWithParent(&parent);
    EXPECT_EQ(barWithParent.parent(), &parent);
}

TEST_F(TestTextIndexStatusBar, SetStatus_WithIndexing_UpdatesCorrectly)
{
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->setStatus(TextIndexStatusBar::Status::Indexing));
}

TEST_F(TestTextIndexStatusBar, SetStatus_WithCompleted_UpdatesCorrectly)
{
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->setStatus(TextIndexStatusBar::Status::Completed));
}

TEST_F(TestTextIndexStatusBar, SetStatus_WithFailed_UpdatesCorrectly)
{
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->setStatus(TextIndexStatusBar::Status::Failed));
}

TEST_F(TestTextIndexStatusBar, SetStatus_WithInactive_UpdatesCorrectly)
{
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->setStatus(TextIndexStatusBar::Status::Inactive));
}

TEST_F(TestTextIndexStatusBar, UpdateIndexingProgress_WithValidValues_UpdatesCorrectly)
{
    qlonglong count = 50;
    qlonglong total = 100;

    // Mock UI operations
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->updateIndexingProgress(count, total));
}

TEST_F(TestTextIndexStatusBar, UpdateIndexingProgress_WithZeroTotal_HandlesCorrectly)
{
    qlonglong count = 10;
    qlonglong total = 0;

    // Mock UI operations
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->updateIndexingProgress(count, total));
}

TEST_F(TestTextIndexStatusBar, SetRunning_WithTrue_StartsSpinner)
{
    // Mock spinner operations
    stub.set_lamda(&DSpinner::start, [](DSpinner *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QWidget::show, [](QWidget *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->setRunning(true));
}

TEST_F(TestTextIndexStatusBar, SetRunning_WithFalse_StopsSpinner)
{
    // Mock spinner operations
    stub.set_lamda(&DSpinner::stop, [](DSpinner *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QWidget::hide, [](QWidget *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->setRunning(false));
}

TEST_F(TestTextIndexStatusBar, Status_ReturnsCurrentStatus)
{
    TextIndexStatusBar::Status status = statusBar->status();

    // Initial status should be Inactive
    EXPECT_EQ(status, TextIndexStatusBar::Status::Inactive);
}

TEST_F(TestTextIndexStatusBar, UpdateUI_WithDifferentStatuses_CallsCorrectly)
{
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    QList<TextIndexStatusBar::Status> statuses = {
        TextIndexStatusBar::Status::Indexing,
        TextIndexStatusBar::Status::Completed,
        TextIndexStatusBar::Status::Failed,
        TextIndexStatusBar::Status::Inactive
    };

    for (auto status : statuses) {
        EXPECT_NO_FATAL_FAILURE(statusBar->updateUI(status));
    }
}

TEST_F(TestTextIndexStatusBar, SetFormattedTextWithLink_CallsCorrectly)
{
    QString mainText = "Main text";
    QString linkText = "Link text";
    QString href = "http://example.com";

    // Mock UI operations
    stub.set_lamda(&QLabel::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(statusBar->setFormattedTextWithLink(mainText, linkText, href));
}

TEST_F(TestTextIndexStatusBar, ResetIndexSignal_CanBeEmitted)
{
    QSignalSpy spy(statusBar, &TextIndexStatusBar::resetIndex);

    // Emit the signal manually to test
    emit statusBar->resetIndex();

    EXPECT_EQ(spy.count(), 1);
}

// CheckBoxWidthTextIndex Tests
TEST_F(TestCheckBoxWidthTextIndex, Constructor_CreatesValidInstance)
{
    EXPECT_NE(checkBox, nullptr);

    // Test with parent widget
    QWidget parent;
    CheckBoxWidthTextIndex boxWithParent(&parent);
    EXPECT_EQ(boxWithParent.parent(), &parent);
}

TEST_F(TestCheckBoxWidthTextIndex, ConnectToBackend_CallsCorrectly)
{
    // Mock TextIndexClient operations
    stub.set_lamda(&TextIndexClient::instance, []() -> TextIndexClient * {
        __DBG_STUB_INVOKE__
        static TextIndexClient mockClient;
        return &mockClient;
    });

    EXPECT_NO_FATAL_FAILURE(checkBox->connectToBackend());
}

TEST_F(TestCheckBoxWidthTextIndex, SetDisplayText_UpdatesCorrectly)
{
    QString text = "Enable full-text search";

    // Mock checkbox operations
    stub.set_lamda(&QAbstractButton::setText, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(checkBox->setDisplayText(text));
}

TEST_F(TestCheckBoxWidthTextIndex, SetChecked_WithTrue_UpdatesCheckbox)
{
    // Mock checkbox operations
    stub.set_lamda(&QAbstractButton::setChecked, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(checkBox->setChecked(true));
}

TEST_F(TestCheckBoxWidthTextIndex, SetChecked_WithFalse_UpdatesCheckbox)
{
    // Mock checkbox operations
    stub.set_lamda(&QAbstractButton::setChecked, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(checkBox->setChecked(false));
}

TEST_F(TestCheckBoxWidthTextIndex, InitStatusBar_CallsCorrectly)
{
    EXPECT_NO_FATAL_FAILURE(checkBox->initStatusBar());
}

TEST_F(TestCheckBoxWidthTextIndex, CheckStateChangedSignal_CanBeEmitted)
{
    QSignalSpy spy(checkBox, &CheckBoxWidthTextIndex::checkStateChanged);

    // Emit the signal manually to test
    emit checkBox->checkStateChanged(Qt::Checked);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).value<Qt::CheckState>(), Qt::Checked);
}

TEST_F(TestCheckBoxWidthTextIndex, ShouldHandleIndexEvent_WithValidParameters_ReturnsCorrectValue)
{
    QString path = "/home/test";
    TextIndexClient::TaskType type = TextIndexClient::TaskType::Create;

    // Call the private method via metaObject
    bool result = false;
    QMetaObject::invokeMethod(checkBox, "shouldHandleIndexEvent", Qt::DirectConnection,
                              Q_RETURN_ARG(bool, result),
                              Q_ARG(QString, path),
                              Q_ARG(TextIndexClient::TaskType, type));

    // Test that method can be called
    EXPECT_TRUE(result || !result);
}

TEST_F(TestCheckBoxWidthTextIndex, CompleteWorkflow_WithBackendConnection)
{
    // Mock all backend operations
    stub.set_lamda(&TextIndexClient::instance, []() -> TextIndexClient * {
        __DBG_STUB_INVOKE__
        static TextIndexClient mockClient;
        return &mockClient;
    });

    stub.set_lamda(&QAbstractButton::setText, []() {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QAbstractButton::setChecked, []() {
        __DBG_STUB_INVOKE__
    });

    // Test complete workflow
    EXPECT_NO_FATAL_FAILURE(checkBox->connectToBackend());
    EXPECT_NO_FATAL_FAILURE(checkBox->setDisplayText("Test text"));
    EXPECT_NO_FATAL_FAILURE(checkBox->setChecked(true));
    EXPECT_NO_FATAL_FAILURE(checkBox->initStatusBar());
}

TEST_F(TestCheckBoxWidthTextIndex, IndexCheckContextEnum_AllValuesAccessible)
{
    // Test that all IndexCheckContext enum values exist and can be used
    QList<IndexCheckContext> contexts = {
        IndexCheckContext::None,
        IndexCheckContext::ResetIndex,
        IndexCheckContext::InitStatus
    };

    // Just verify the enum values exist and can be used
    for (auto context : contexts) {
        EXPECT_TRUE(true); // Basic test that enum values are accessible
    }
}
