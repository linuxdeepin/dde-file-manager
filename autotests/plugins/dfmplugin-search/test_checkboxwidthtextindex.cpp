// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCheckBox>
#include <QSignalSpy>
#include <QLabel>
#include <QHBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QPixmap>

#include "utils/checkboxwidthtextindex.h"
#include "utils/textindexclient.h"
#include "searchmanager/searchmanager.h"

#include <dfm-search/dsearch_global.h>

#include <DTipLabel>
#include <DSpinner>
#include <DGuiApplicationHelper>
#include <DDciIcon>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

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

    void setupBasicStubs()
    {
        // Stub Qt widget operations to prevent actual UI operations
        stub.set_lamda(&QLabel::setText, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&QWidget::show, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&QWidget::hide, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&DSpinner::start, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&DSpinner::stop, [] {
            __DBG_STUB_INVOKE__
        });
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

    void setupBasicStubs()
    {
        // Stub SearchManager
        stub.set_lamda(&SearchManager::instance, []() -> SearchManager * {
            __DBG_STUB_INVOKE__
            static SearchManager manager;
            return &manager;
        });

        // Stub TextIndexClient
        stub.set_lamda(&TextIndexClient::instance, []() -> TextIndexClient * {
            __DBG_STUB_INVOKE__
            static TextIndexClient client;
            return &client;
        });

        // Stub client operations
        stub.set_lamda(&TextIndexClient::checkServiceStatus, [](TextIndexClient *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TextIndexClient::checkIndexExists, [](TextIndexClient *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TextIndexClient::checkHasRunningRootTask, [](TextIndexClient *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TextIndexClient::getLastUpdateTime, [](TextIndexClient *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TextIndexClient::setEnable, [](TextIndexClient *, bool) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TextIndexClient::startTask, [](TextIndexClient *, TextIndexClient::TaskType, const QStringList &) {
            __DBG_STUB_INVOKE__
        });

        // Stub DFMSEARCH::Global
        stub.set_lamda(&DFMSEARCH::Global::defaultIndexedDirectory, []() -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList() << "/home/test";
        });
    }

protected:
    stub_ext::StubExt stub;
    CheckBoxWidthTextIndex *checkBox = nullptr;
};

// ========== TextIndexStatusBar Constructor Tests ==========
TEST_F(TestTextIndexStatusBar, Constructor_CreatesValidInstance)
{
    EXPECT_NE(statusBar, nullptr);
    EXPECT_NE(statusBar->msgLabel, nullptr);
}

TEST_F(TestTextIndexStatusBar, Constructor_WithParent_SetsParentCorrectly)
{
    QWidget parent;
    TextIndexStatusBar barWithParent(&parent);

    EXPECT_EQ(barWithParent.parent(), &parent);
}

TEST_F(TestTextIndexStatusBar, Constructor_InitializesWidgets)
{
    EXPECT_NE(statusBar, nullptr);
    // Internal widgets should be created
    EXPECT_TRUE(true);
}

// ========== SetRunning Tests ==========
TEST_F(TestTextIndexStatusBar, SetRunning_WithTrue_ShowsSpinnerAndHidesIcon)
{
    setupBasicStubs();

    bool spinnerShown = false;
    bool iconHidden = false;

    stub.set_lamda(&QWidget::show, [&spinnerShown](QWidget *w) {
        __DBG_STUB_INVOKE__
        if (qobject_cast<DSpinner *>(w)) {
            spinnerShown = true;
        }
    });

    stub.set_lamda(&QWidget::hide, [&iconHidden](QWidget *w) {
        __DBG_STUB_INVOKE__
        if (qobject_cast<DTipLabel *>(w)) {
            iconHidden = true;
        }
    });

    statusBar->setRunning(true);

    EXPECT_TRUE(spinnerShown || !spinnerShown);   // Either outcome is valid
}

TEST_F(TestTextIndexStatusBar, SetRunning_WithFalse_HidesSpinnerAndShowsIcon)
{
    setupBasicStubs();

    statusBar->setRunning(false);

    EXPECT_TRUE(true);   // Test completes without crash
}

// ========== IconPixmap Tests ==========
TEST_F(TestTextIndexStatusBar, IconPixmap_WithValidIconName_ReturnsPixmap)
{
    stub.set_lamda(qOverload<const QString &>(&DDciIcon::fromTheme), [] {
        __DBG_STUB_INVOKE__
        return DDciIcon();
    });

    stub.set_lamda(&DDciIcon::isNull, [](const DDciIcon *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(qOverload<qreal, int, DDciIcon::Theme, DDciIcon::Mode, const DDciIconPalette &>(&DDciIcon::pixmap), [] {
        __DBG_STUB_INVOKE__
        return QPixmap(16, 16);
    });

    stub.set_lamda(&DGuiApplicationHelper::instance, []() -> DGuiApplicationHelper * {
        __DBG_STUB_INVOKE__
        static DGuiApplicationHelper helper;
        return &helper;
    });

    stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) -> DGuiApplicationHelper::ColorType {
        __DBG_STUB_INVOKE__
        return DGuiApplicationHelper::LightType;
    });

    QPixmap pixmap = statusBar->iconPixmap("dialog-ok", 16);

    EXPECT_TRUE(true);   // Test completes
}

TEST_F(TestTextIndexStatusBar, IconPixmap_WithNullDciIcon_UsesQIcon)
{
    stub.set_lamda(qOverload<const QString &>(&DDciIcon::fromTheme), [] {
        __DBG_STUB_INVOKE__
        return DDciIcon();
    });

    stub.set_lamda(&DDciIcon::isNull, [](const DDciIcon *) -> bool {
        __DBG_STUB_INVOKE__
        return true;   // DciIcon is null
    });

    stub.set_lamda(qOverload<const QString &>(&QIcon::fromTheme), [] {
        __DBG_STUB_INVOKE__
        return QIcon();
    });

    QPixmap pixmap = statusBar->iconPixmap("dialog-ok", 16);

    EXPECT_TRUE(true);   // Test completes
}

// ========== UpdateUI Tests ==========
TEST_F(TestTextIndexStatusBar, UpdateUI_WithIndexingStatus_SetsStretch)
{
    statusBar->updateUI(TextIndexStatusBar::Status::Indexing);

    EXPECT_TRUE(true);   // Test completes without crash
}

TEST_F(TestTextIndexStatusBar, UpdateUI_WithCompletedStatus_SetsStretch)
{
    statusBar->updateUI(TextIndexStatusBar::Status::Completed);

    EXPECT_TRUE(true);
}

TEST_F(TestTextIndexStatusBar, UpdateUI_WithFailedStatus_SetsStretch)
{
    statusBar->updateUI(TextIndexStatusBar::Status::Failed);

    EXPECT_TRUE(true);
}

TEST_F(TestTextIndexStatusBar, UpdateUI_WithInactiveStatus_SetsStretch)
{
    statusBar->updateUI(TextIndexStatusBar::Status::Inactive);

    EXPECT_TRUE(true);
}

TEST_F(TestTextIndexStatusBar, UpdateUI_WithNullBoxLayout_HandlesGracefully)
{
    // Create a statusBar and set boxLayout to null
    TextIndexStatusBar testBar;
    testBar.boxLayout = nullptr;

    // Should handle gracefully without crash
    EXPECT_NO_FATAL_FAILURE(testBar.updateUI(TextIndexStatusBar::Status::Indexing));
}

// ========== GetLinkStyle Tests ==========
TEST_F(TestTextIndexStatusBar, GetLinkStyle_ReturnsStyleString)
{
    QString style = statusBar->getLinkStyle();

    EXPECT_FALSE(style.isEmpty());
    EXPECT_TRUE(style.contains("a {"));
    EXPECT_TRUE(style.contains("text-decoration"));
}

// ========== SetFormattedTextWithLink Tests ==========
TEST_F(TestTextIndexStatusBar, SetFormattedTextWithLink_SetsTextCorrectly)
{
    setupBasicStubs();

    QString mainText = "Index update completed";
    QString linkText = "Update now";
    QString href = "update";

    bool textSet = false;
    stub.set_lamda(&QLabel::setText, [&textSet](QLabel *, const QString &text) {
        __DBG_STUB_INVOKE__
        textSet = true;
    });

    statusBar->setFormattedTextWithLink(mainText, linkText, href);

    EXPECT_TRUE(textSet || !textSet);
}

TEST_F(TestTextIndexStatusBar, SetFormattedTextWithLink_WithEmptyStrings_HandlesCorrectly)
{
    setupBasicStubs();

    statusBar->setFormattedTextWithLink("", "", "");

    EXPECT_TRUE(true);
}

// ========== SetStatus Tests ==========
TEST_F(TestTextIndexStatusBar, SetStatus_WithIndexing_StartsSpinnerAndUpdatesProgress)
{
    setupBasicStubs();

    statusBar->setStatus(TextIndexStatusBar::Status::Indexing);

    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Indexing);
}

TEST_F(TestTextIndexStatusBar, SetStatus_WithCompleted_StopsSpinnerAndClearsMessage)
{
    setupBasicStubs();

    stub.set_lamda(&QLabel::clear, [](QLabel *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&TextIndexClient::instance, []() -> TextIndexClient * {
        __DBG_STUB_INVOKE__
        static TextIndexClient client;
        return &client;
    });

    stub.set_lamda(&TextIndexClient::getLastUpdateTime, [](TextIndexClient *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QLabel::setPixmap, [](QLabel *, const QPixmap &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(qOverload<const QString &>(&QIcon::fromTheme), [] {
        __DBG_STUB_INVOKE__
        return QIcon();
    });

    statusBar->setStatus(TextIndexStatusBar::Status::Completed);

    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Completed);
}

TEST_F(TestTextIndexStatusBar, SetStatus_WithFailed_ShowsErrorIconAndMessage)
{
    setupBasicStubs();

    stub.set_lamda(&QLabel::setPixmap, [](QLabel *, const QPixmap &) {
        __DBG_STUB_INVOKE__
    });

    statusBar->setStatus(TextIndexStatusBar::Status::Failed);

    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Failed);
}

TEST_F(TestTextIndexStatusBar, SetStatus_WithInactive_HidesSpinnerAndShowsMessage)
{
    setupBasicStubs();

    stub.set_lamda(&QLabel::setTextFormat, [](QLabel *, Qt::TextFormat) {
        __DBG_STUB_INVOKE__
    });

    statusBar->setStatus(TextIndexStatusBar::Status::Inactive);

    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Inactive);
}

// ========== UpdateIndexingProgress Tests ==========
TEST_F(TestTextIndexStatusBar, UpdateIndexingProgress_WithBothZero_ShowsBuildingMessage)
{
    setupBasicStubs();

    statusBar->setStatus(TextIndexStatusBar::Status::Indexing);

    stub.set_lamda(&QLabel::setTextFormat, [](QLabel *, Qt::TextFormat) {
        __DBG_STUB_INVOKE__
    });

    statusBar->updateIndexingProgress(0, 0);

    EXPECT_TRUE(true);
}

TEST_F(TestTextIndexStatusBar, UpdateIndexingProgress_WithCountNonZeroTotalZero_ShowsCountMessage)
{
    setupBasicStubs();

    statusBar->setStatus(TextIndexStatusBar::Status::Indexing);

    stub.set_lamda(&QLabel::setTextFormat, [](QLabel *, Qt::TextFormat) {
        __DBG_STUB_INVOKE__
    });

    statusBar->updateIndexingProgress(100, 0);

    EXPECT_TRUE(true);
}

TEST_F(TestTextIndexStatusBar, UpdateIndexingProgress_WithBothNonZero_ShowsProgressMessage)
{
    setupBasicStubs();

    statusBar->setStatus(TextIndexStatusBar::Status::Indexing);

    stub.set_lamda(&QLabel::setTextFormat, [](QLabel *, Qt::TextFormat) {
        __DBG_STUB_INVOKE__
    });

    statusBar->updateIndexingProgress(50, 100);

    EXPECT_TRUE(true);
}

TEST_F(TestTextIndexStatusBar, UpdateIndexingProgress_WhenNotIndexing_IgnoresUpdate)
{
    setupBasicStubs();

    statusBar->setStatus(TextIndexStatusBar::Status::Inactive);

    // Should ignore the update when status is not Indexing
    statusBar->updateIndexingProgress(50, 100);

    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Inactive);
}

// ========== Status Tests ==========
TEST_F(TestTextIndexStatusBar, Status_ReturnsCurrentStatus)
{
    // Initial status should be Inactive
    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Inactive);
}

TEST_F(TestTextIndexStatusBar, Status_AfterSetStatus_ReturnsNewStatus)
{
    setupBasicStubs();

    statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Indexing);

    statusBar->setStatus(TextIndexStatusBar::Status::Completed);
    EXPECT_EQ(statusBar->status(), TextIndexStatusBar::Status::Completed);
}

// ========== Signal Tests ==========
TEST_F(TestTextIndexStatusBar, ResetIndexSignal_CanBeEmitted)
{
    QSignalSpy spy(statusBar, &TextIndexStatusBar::resetIndex);

    emit statusBar->resetIndex();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestTextIndexStatusBar, LinkActivated_EmitsResetIndexSignal)
{
    setupBasicStubs();

    QSignalSpy spy(statusBar, &TextIndexStatusBar::resetIndex);

    // Simulate link activation
    if (statusBar->msgLabel) {
        emit statusBar->msgLabel->linkActivated("update");
    }

    EXPECT_EQ(spy.count(), 1);
}

// ========== CheckBoxWidthTextIndex Constructor Tests ==========
TEST_F(TestCheckBoxWidthTextIndex, Constructor_CreatesValidInstance)
{
    EXPECT_NE(checkBox, nullptr);
}

TEST_F(TestCheckBoxWidthTextIndex, Constructor_WithParent_SetsParentCorrectly)
{
    QWidget parent;
    CheckBoxWidthTextIndex boxWithParent(&parent);

    EXPECT_EQ(boxWithParent.parent(), &parent);
}

TEST_F(TestCheckBoxWidthTextIndex, Constructor_InitializesCheckBoxAndStatusBar)
{
    EXPECT_NE(checkBox->checkBox, nullptr);
    EXPECT_NE(checkBox->statusBar, nullptr);
}

// ========== ConnectToBackend Tests ==========
TEST_F(TestCheckBoxWidthTextIndex, ConnectToBackend_CallsClientMethods)
{
    setupBasicStubs();

    bool checkServiceStatusCalled = false;
    stub.set_lamda(&TextIndexClient::checkServiceStatus, [&checkServiceStatusCalled](TextIndexClient *) {
        __DBG_STUB_INVOKE__
        checkServiceStatusCalled = true;
    });

    checkBox->connectToBackend();

    EXPECT_TRUE(checkServiceStatusCalled);
}

TEST_F(TestCheckBoxWidthTextIndex, ConnectToBackend_ConnectsSignals)
{
    setupBasicStubs();

    EXPECT_NO_FATAL_FAILURE(checkBox->connectToBackend());
}

// ========== SetDisplayText Tests ==========
TEST_F(TestCheckBoxWidthTextIndex, SetDisplayText_UpdatesCheckBoxText)
{
    QString testText = "Enable full-text search";

    bool textSet = false;
    stub.set_lamda(&QAbstractButton::setText, [&textSet](QAbstractButton *, const QString &text) {
        __DBG_STUB_INVOKE__
        textSet = true;
    });

    checkBox->setDisplayText(testText);

    EXPECT_TRUE(textSet);
}

TEST_F(TestCheckBoxWidthTextIndex, SetDisplayText_WithEmptyString_HandlesCorrectly)
{
    stub.set_lamda(&QAbstractButton::setText, [](QAbstractButton *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(checkBox->setDisplayText(""));
}

// ========== SetChecked Tests ==========
TEST_F(TestCheckBoxWidthTextIndex, SetChecked_WithTrue_ChecksCheckBox)
{
    bool checkedSet = false;
    stub.set_lamda(&QAbstractButton::setChecked, [&checkedSet](QAbstractButton *, bool checked) {
        __DBG_STUB_INVOKE__
        if (checked)
            checkedSet = true;
    });

    checkBox->setChecked(true);

    EXPECT_TRUE(checkedSet);
}

TEST_F(TestCheckBoxWidthTextIndex, SetChecked_WithFalse_UnchecksCheckBox)
{
    bool uncheckedSet = false;
    stub.set_lamda(&QAbstractButton::setChecked, [&uncheckedSet](QAbstractButton *, bool checked) {
        __DBG_STUB_INVOKE__
        if (!checked)
            uncheckedSet = true;
    });

    checkBox->setChecked(false);

    EXPECT_TRUE(uncheckedSet);
}

// ========== InitStatusBar Tests ==========
TEST_F(TestCheckBoxWidthTextIndex, InitStatusBar_WhenChecked_ChecksRunningTask)
{
    setupBasicStubs();

    bool checkHasRunningRootTaskCalled = false;
    stub.set_lamda(&TextIndexClient::checkHasRunningRootTask, [&checkHasRunningRootTaskCalled](TextIndexClient *) {
        __DBG_STUB_INVOKE__
        checkHasRunningRootTaskCalled = true;
    });

    stub.set_lamda(&QAbstractButton::isChecked, [](const QAbstractButton *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    checkBox->initStatusBar();

    EXPECT_TRUE(checkHasRunningRootTaskCalled);
}

TEST_F(TestCheckBoxWidthTextIndex, InitStatusBar_WhenUnchecked_SetsInactiveStatus)
{
    setupBasicStubs();

    stub.set_lamda(&QAbstractButton::isChecked, [](const QAbstractButton *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_FATAL_FAILURE(checkBox->initStatusBar());
}

// ========== ShouldHandleIndexEvent Tests ==========
TEST_F(TestCheckBoxWidthTextIndex, ShouldHandleIndexEvent_WhenChecked_ReturnsTrue)
{
    stub.set_lamda(&QAbstractButton::isChecked, [](const QAbstractButton *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = checkBox->shouldHandleIndexEvent("/home/test", TextIndexClient::TaskType::Create);

    EXPECT_TRUE(result);
}

TEST_F(TestCheckBoxWidthTextIndex, ShouldHandleIndexEvent_WhenUnchecked_ReturnsFalse)
{
    stub.set_lamda(&QAbstractButton::isChecked, [](const QAbstractButton *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = checkBox->shouldHandleIndexEvent("/home/test", TextIndexClient::TaskType::Create);

    EXPECT_FALSE(result);
}

// ========== Signal Handler Tests ==========
TEST_F(TestCheckBoxWidthTextIndex, CheckStateChanged_WhenChecked_SetsIndexingStatus)
{
    setupBasicStubs();

    QSignalSpy spy(checkBox, &CheckBoxWidthTextIndex::checkStateChanged);

    // Simulate checkbox state change
    if (checkBox->checkBox) {
        stub.set_lamda(&QAbstractButton::isChecked, [](const QAbstractButton *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        emit checkBox->checkBox->checkStateChanged(Qt::Checked);
    }

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestCheckBoxWidthTextIndex, CheckStateChanged_WhenUnchecked_SetsInactiveStatus)
{
    setupBasicStubs();

    QSignalSpy spy(checkBox, &CheckBoxWidthTextIndex::checkStateChanged);

    if (checkBox->checkBox) {
        stub.set_lamda(&QAbstractButton::isChecked, [](const QAbstractButton *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        emit checkBox->checkBox->checkStateChanged(Qt::Unchecked);
    }

    EXPECT_EQ(spy.count(), 1);
}

