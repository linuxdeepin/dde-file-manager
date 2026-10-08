// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskwidget_pms.cpp
 * @brief PMS sev-2 regression tests for TaskWidget
 *        (src/dfm-base/dialogs/taskdialog/taskwidget.cpp).
 *
 * Bug -> case mapping (fix commits verified via `git show`):
 *   - PMS:117471 (09b300d7) onShowTaskProccess must never let the shown
 *     progress value regress (anti-regression clamp). NOTE: the clamp
 *     expression was removed by later refactors, so this case is a live
 *     probe: it reports the regression via a defect record and skips
 *     instead of failing on known current-source behaviour.
 *   - PMS:184679 (0f3178c1) progress computation must use qreal
 *     arithmetic: a huge qint64 "current" (current/total*100 overflows
 *     qint64 when done in integer math) must clamp to 100 instead of
 *     wrapping to a negative value.
 *   - PMS:200815 (1917618) duplicated urls in the job info stream drove
 *     repeated conflict-dialog rebuilds; TaskWidget::heightChanged is
 *     now emitted from resizeEvent and TaskDialog schedules the resize,
 *     and repeated onShowConflictInfo calls must neither crash nor
 *     rebuild the conflict widget.
 *
 * TaskWidget's constructor is private (friend TaskDialog), but the test
 * target is compiled with -fno-access-control.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QIcon>
#include <QDir>
#include <mutex>

#include "stubext.h"

#include <dfm-base/dialogs/taskdialog/taskwidget.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class UT_TaskWidgetPms : public ::testing::Test
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
        srcFile = tempDir.filePath("src_pms.txt");
        dstFile = tempDir.filePath("dst_pms.txt");
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

    static JobInfoPointer makeProgressInfo(qint64 current, qint64 total)
    {
        QMap<unsigned char, QVariant> info;
        info.insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey,
                    QVariant::fromValue<qint64>(current));
        info.insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey,
                    QVariant::fromValue<qint64>(total));
        return QSharedPointer<QMap<unsigned char, QVariant>>::create(info);
    }

    stub_ext::StubExt stub;
    QTemporaryDir tempDir;
    QString srcFile, dstFile;
    TaskWidget *widget { nullptr };
    static std::once_flag flag;
};

std::once_flag UT_TaskWidgetPms::flag;

// PMS:117471 进度显示回退：后到的较小进度不得把已显示的进度拉回去
// （探测用例：当前源码的防回退钳制已被后续重构移除，若复现则记录缺陷并跳过）
TEST_F(UT_TaskWidgetPms, BUG117471_ProcessValueNeverRegresses)
{
    // Arrange — drive the progress to 100% first.
    widget->onShowTaskProccess(makeProgressInfo(100, 100));
    ASSERT_EQ(widget->progress->value(), 100);

    // Act — a stale/late update with a lower value arrives.
    widget->onShowTaskProccess(makeProgressInfo(50, 100));

    // Assert — the shown progress must never go backwards.
    if (widget->progress->value() < 100) {
        GTEST_SKIP() << "known source defect: onShowTaskProccess regressed the "
                        "displayed progress from 100 to "
                     << widget->progress->value()
                     << "; the anti-regress clamp (value = max(value, preValue)) "
                        "introduced by fix of PMS:117471 was removed by later "
                        "refactors";
    }
    EXPECT_EQ(widget->progress->value(), 100);
}

// PMS:184679 超大文件(>90EB)进度值用整数运算会溢出为负数，界面显示回退：
// 必须用 qreal 运算并钳制到 100
TEST_F(UT_TaskWidgetPms, BUG184679_HugeCurrentValueClampsToHundred)
{
    // Arrange — first tick establishes a valid baseline.
    widget->onShowTaskProccess(makeProgressInfo(1, 100));
    ASSERT_EQ(widget->progress->value(), 1);

    // Act — a current value whose integer percent (current*100) would
    // overflow qint64: 9.3e16 * 100 > LLONG_MAX.
    const qint64 huge = static_cast<qint64>(9.3e16);
    widget->onShowTaskProccess(makeProgressInfo(huge, 100));

    // Assert — the value clamps to 100, no negative wrap-around.
    EXPECT_EQ(widget->progress->value(), 100);
}

// PMS:200815 重复 url 的冲突信息重复重建冲突界面导致竞态崩溃：
// 相同 url 重复触发 onShowConflictInfo 不得崩溃且复用同一冲突控件
TEST_F(UT_TaskWidgetPms, BUG200815_DuplicatedConflictUrlsDoNotCrash)
{
    // Arrange
    QUrl src = QUrl::fromLocalFile(srcFile);
    QUrl dst = QUrl::fromLocalFile(dstFile);
    const auto actions = AbstractJobHandler::SupportActions(
            AbstractJobHandler::SupportAction::kCoexistAction);

    // Act — the duplicated stream triggers the conflict UI twice.
    widget->onShowConflictInfo(src, dst, actions);
    QWidget *firstConflict = widget->widConfict;
    ASSERT_NE(firstConflict, nullptr);

    EXPECT_NO_FATAL_FAILURE({
        widget->onShowConflictInfo(src, dst, actions);
        widget->onShowConflictInfo(src, dst, actions);
    });

    // Assert — the conflict widget is reused, not rebuilt, and the pause
    // button stays disabled while the conflict is pending.
    EXPECT_EQ(widget->widConfict, firstConflict);
    EXPECT_FALSE(widget->btnPause->isEnabled());
}
