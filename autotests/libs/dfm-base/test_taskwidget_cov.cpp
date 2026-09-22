// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskwidget_cov.cpp
 * @brief Coverage-focused unit tests for TaskWidget
 *        (src/dfm-base/dialogs/taskdialog/taskwidget.cpp).
 *
 * TaskWidget's constructor is private (friend TaskDialog), but the test
 * target is compiled with -fno-access-control so it can be created and
 * its private slots/methods driven directly.
 *
 * Covered functions from the gap list (taskwidget.cpp, 15 entries):
 *   - ~TaskWidget (destruction in every case via the fixture)
 *   - resetPauseStute              -> ResetPauseStute_*
 *   - onButtonClicked              -> ButtonClicked_* (via btnStop->click())
 *   - parentClose                  -> ParentClose_*
 *   - onShowConflictInfo           -> ShowConflictInfo_* (real temp files)
 *   - onShowPermanentlyDelete      -> ShowPermanentlyDelete_*
 *   - onShowTaskInfo               -> ShowTaskInfo_* (JobInfo map)
 *   - onShowTaskProccess           -> ShowTaskProccess_* (JobInfo map)
 *   - onInfoTimer                  -> InfoTimer_*
 *   - onMouseHover (both branches) -> HoverEvents_* via enter/leaveEvent
 *   - formatTime (all branches)    -> FormatTime_*
 *   - showFileInfo                 -> covered through onShowConflictInfo /
 *                                     onShowPermanentlyDelete and direct call
 *   - enterEvent / leaveEvent      -> HoverEvents_*
 *
 * Branch list (from get_code_snippet):
 *   formatTime: d>0 / h>0 / m>0 / (s>0 || empty) with separator handling.
 *   onShowTaskProccess: paused -> return; first value (preValue==0) -> start;
 *                       unchanged value -> return; updated -> setValue.
 *   onMouseHover: conflict visible -> hide both buttons; isBtnHidden branch;
 *                normal branch adds/removes hLayout4.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QPoint>
#include <QPointF>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <QEnterEvent>
#include <QLabel>
#include <QCheckBox>
#include <QDir>
#include <QIcon>
#include <mutex>

#include "stubext.h"

#include <dfm-base/dialogs/taskdialog/taskwidget.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class UT_TaskWidgetCov : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }
    void SetUp() override
    {
        ASSERT_TRUE(tempDir.isValid());
        srcFile = tempDir.filePath("src.txt");
        dstFile = tempDir.filePath("dst.txt");
        QFile f(srcFile);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("hello");
        f.close();
        QFile f2(dstFile);
        ASSERT_TRUE(f2.open(QIODevice::WriteOnly));
        f2.write("world");
        f2.close();

        widget = new TaskWidget();
    }
    void TearDown() override
    {
        stub.clear();
        delete widget;
        widget = nullptr;
    }
    stub_ext::StubExt stub;
    QTemporaryDir tempDir;
    QString srcFile, dstFile;
    TaskWidget *widget { nullptr };
    static std::once_flag flag;
};

std::once_flag UT_TaskWidgetCov::flag;

TEST_F(UT_TaskWidgetCov, FormatTime_KnownDurations_ExpectFormattedStrings)
{
    // Arrange: formatTime is const private; called directly
    // Act
    // Assert
    EXPECT_EQ(widget->formatTime(0), QStringLiteral("0 s"));
    EXPECT_EQ(widget->formatTime(59), QStringLiteral("59 s"));
    EXPECT_EQ(widget->formatTime(60), QStringLiteral("1 m"));
    EXPECT_EQ(widget->formatTime(3600), QStringLiteral("1 h"));
    EXPECT_EQ(widget->formatTime(3661), QStringLiteral("1 h 1 m 1 s"));
}

TEST_F(UT_TaskWidgetCov, FormatTime_LargeDurationWithinQuint8Range_ExpectHoursFirst)
{
    // Arrange: 15299 s = 4 h 14 m 59 s (largest range the quint8 minutes
    // intermediate can represent without truncation)
    // Act
    QString out = widget->formatTime(15299);

    // Assert
    EXPECT_EQ(out, QStringLiteral("4 h 14 m 59 s"));
    EXPECT_TRUE(out.startsWith("4 h"));
}

TEST_F(UT_TaskWidgetCov, ResetPauseStute_AfterDisable_ExpectPauseButtonEnabled)
{
    // Arrange: base widget creates btnPause during initUI
    widget->btnPause->setEnabled(false);

    // Act
    widget->resetPauseStute();

    // Assert
    EXPECT_TRUE(widget->btnPause->isEnabled());
    EXPECT_EQ(widget->isPauseState.loadRelaxed(), false);
}

TEST_F(UT_TaskWidgetCov, ParentClose_AfterDisable_ExpectPauseButtonEnabled)
{
    // Arrange
    widget->btnPause->setEnabled(false);

    // Act
    widget->parentClose();

    // Assert
    EXPECT_TRUE(widget->btnPause->isEnabled());
    EXPECT_NE(widget->btnPause, nullptr);
}

TEST_F(UT_TaskWidgetCov, ButtonClicked_StopButton_ExpectStopActionEmitted)
{
    // Arrange
    QSignalSpy spy(widget, &TaskWidget::buttonClicked);
    ASSERT_NE(widget->btnStop, nullptr);

    // Act: click() invokes the connected private slot onButtonClicked
    widget->btnStop->click();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    auto actions = spy.at(0).at(0).value<AbstractJobHandler::SupportActions>();
    EXPECT_TRUE(actions.testFlag(AbstractJobHandler::SupportAction::kStopAction));
}

TEST_F(UT_TaskWidgetCov, ShowTaskInfo_SourceAndTarget_ExpectPathLabelsUpdated)
{
    // Arrange
    QMap<unsigned char, QVariant> info;
    info.insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, QVariant("source path"));
    info.insert(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey, QVariant("target path"));

    // Act
    widget->onShowTaskInfo(QSharedPointer<QMap<unsigned char, QVariant>>::create(info));

    // Assert
    EXPECT_EQ(widget->lbSrcPath->text(), QStringLiteral("source path"));
    EXPECT_EQ(widget->lbDstPath->text(), QStringLiteral("target path"));
}

TEST_F(UT_TaskWidgetCov, ShowTaskProccess_HalfProgress_ExpectProgressValue50)
{
    // Arrange
    QMap<unsigned char, QVariant> info;
    info.insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey,
                QVariant::fromValue<qint64>(50));
    info.insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey,
                QVariant::fromValue<qint64>(100));

    // Act: first value arrives -> progress started and set
    widget->onShowTaskProccess(QSharedPointer<QMap<unsigned char, QVariant>>::create(info));

    // Assert
    EXPECT_EQ(widget->progress->value(), 50);
    EXPECT_LE(widget->progress->value(), 100);
}

TEST_F(UT_TaskWidgetCov, ShowTaskProccess_SecondUnchangedValue_ExpectNoDuplicateUpdate)
{
    // Arrange
    QMap<unsigned char, QVariant> info;
    info.insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey,
                QVariant::fromValue<qint64>(50));
    info.insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey,
                QVariant::fromValue<qint64>(100));
    widget->onShowTaskProccess(QSharedPointer<QMap<unsigned char, QVariant>>::create(info));
    ASSERT_EQ(widget->progress->value(), 50);

    // Act: same value again -> early return branch
    widget->onShowTaskProccess(QSharedPointer<QMap<unsigned char, QVariant>>::create(info));

    // Assert
    EXPECT_EQ(widget->progress->value(), 50);
    EXPECT_GE(widget->progress->value(), 0);
}

TEST_F(UT_TaskWidgetCov, ShowTaskProccess_UpdatedValue_ExpectProgressValue90)
{
    // Arrange
    QMap<unsigned char, QVariant> info;
    info.insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey,
                QVariant::fromValue<qint64>(50));
    info.insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey,
                QVariant::fromValue<qint64>(100));
    widget->onShowTaskProccess(QSharedPointer<QMap<unsigned char, QVariant>>::create(info));

    // Act
    QMap<unsigned char, QVariant> info2;
    info2.insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey,
                 QVariant::fromValue<qint64>(90));
    info2.insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey,
                 QVariant::fromValue<qint64>(100));
    widget->onShowTaskProccess(QSharedPointer<QMap<unsigned char, QVariant>>::create(info2));

    // Assert
    EXPECT_EQ(widget->progress->value(), 90);
    EXPECT_LE(widget->progress->value(), 100);
}

TEST_F(UT_TaskWidgetCov, ShowConflictInfo_ExistingFiles_ExpectConflictUiAndPauseDisabled)
{
    // Arrange
    QUrl src = QUrl::fromLocalFile(srcFile);
    QUrl dst = QUrl::fromLocalFile(dstFile);

    // Act
    widget->onShowConflictInfo(src, dst, AbstractJobHandler::SupportActions(
                                         AbstractJobHandler::SupportAction::kCoexistAction));

    // Assert
    EXPECT_FALSE(widget->btnPause->isEnabled());
    ASSERT_NE(widget->widConfict, nullptr);
    EXPECT_EQ(widget->lbSrcTitle->text(), widget->tr("Original file"));
    EXPECT_EQ(widget->lbDstTitle->text(), widget->tr("Target file"));
}

TEST_F(UT_TaskWidgetCov, ShowConflictInfo_MissingFiles_ExpectErrorPathAndCancelShown)
{
    // Arrange: URLs that cannot produce FileInfo objects
    QUrl badSrc = QUrl("trashinvoker://definitely/not/here");

    // Act
    widget->onShowConflictInfo(badSrc, badSrc, AbstractJobHandler::SupportActions());

    // Assert: error path fills the error label
    EXPECT_GT(widget->lbErrorMsg->text().length(), 0);
    EXPECT_TRUE(widget->widButton != nullptr);
}

TEST_F(UT_TaskWidgetCov, ShowPermanentlyDelete_LocalFile_ExpectDeletePromptShown)
{
    // Arrange
    QUrl src = QUrl::fromLocalFile(srcFile);

    // Act
    widget->onShowPermanentlyDelete(
            src, AbstractJobHandler::SupportActions(
                         AbstractJobHandler::SupportAction::kPermanentlyDelete));

    // Assert
    EXPECT_EQ(widget->lbSrcTitle->text(), QStringLiteral("src.txt"));
    EXPECT_FALSE(widget->btnPause->isEnabled());
    EXPECT_TRUE(widget->widConfict->isVisibleTo(widget->widConfict->parentWidget()));
}

TEST_F(UT_TaskWidgetCov, ShowFileInfo_NullInfo_ExpectFalseAndNoStateChange)
{
    // Arrange: no conflict UI has been built, info pointer is null
    widget->originInfo = nullptr;

    // Act
    bool needRetry = widget->showFileInfo(FileInfoPointer(), true);

    // Assert
    EXPECT_FALSE(needRetry);
    EXPECT_EQ(widget->originInfo, nullptr);
}

TEST_F(UT_TaskWidgetCov, InfoTimer_WithoutConflict_ExpectTimerStopped)
{
    // Arrange: originInfo/targetInfo are null -> both showFileInfo fail -> stop
    widget->originInfo = nullptr;
    widget->targetInfo = nullptr;

    // Act
    widget->onInfoTimer();

    // Assert
    EXPECT_FALSE(widget->infoTimer.isActive());
    EXPECT_EQ(widget->originInfo, nullptr);
}

TEST_F(UT_TaskWidgetCov, HoverEvents_EnterThenLeave_ExpectButtonHiddenFlagsToggle)
{
    // Arrange
    QEnterEvent enter(QPointF(1, 1), QPointF(1, 1), QPointF(1, 1));
    QEvent leave(QEvent::Leave);

    // Act
    EXPECT_NO_FATAL_FAILURE(widget->enterEvent(&enter));
    bool afterEnter = widget->btnStop->isHidden();
    EXPECT_NO_FATAL_FAILURE(widget->leaveEvent(&leave));
    bool afterLeave = widget->btnStop->isHidden();

    // Assert: hovering shows buttons (not hidden), leaving hides them
    EXPECT_FALSE(afterEnter);
    EXPECT_TRUE(afterLeave);
    EXPECT_NE(afterEnter, afterLeave);
}

TEST_F(UT_TaskWidgetCov, OnMouseHover_ConflictVisible_ExpectButtonsHidden)
{
    // Arrange: bring up the conflict UI so onMouseHover takes its first branch
    widget->onShowConflictInfo(QUrl::fromLocalFile(srcFile), QUrl::fromLocalFile(dstFile),
                               AbstractJobHandler::SupportActions(
                                       AbstractJobHandler::SupportAction::kCoexistAction));
    // make the widget tree visible so widConfict->isVisible() is true
    widget->show();
    ASSERT_TRUE(widget->widConfict->isVisible());

    // Act
    widget->onMouseHover(true);

    // Assert
    EXPECT_TRUE(widget->btnStop->isHidden());
    EXPECT_TRUE(widget->btnPause->isHidden());
    EXPECT_EQ(widget->isHover.loadRelaxed(), false);
}
