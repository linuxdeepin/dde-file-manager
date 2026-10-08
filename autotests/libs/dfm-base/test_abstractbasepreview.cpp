// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractbasepreview.cpp
 * @brief Unit tests for AbstractBasePreview default implementations.
 */

#include <gtest/gtest.h>
#include <QWidget>
#include <QUrl>
#include <Qt>

#include <dfm-base/interfaces/abstractbasepreview.h>

using namespace dfmbase;

// Minimal concrete subclass to exercise AbstractBasePreview's default virtual methods.
class TestAbstractBasePreview : public AbstractBasePreview
{
public:
    explicit TestAbstractBasePreview(QObject *parent = nullptr)
        : AbstractBasePreview(parent) { }
    bool setFileUrl(const QUrl &url) override { m_url = url; return true; }
    QUrl fileUrl() const override { return m_url; }
    QWidget *contentWidget() const override { return &m_widget; }

private:
    mutable QWidget m_widget;
    QUrl m_url;
};

TEST(AbstractBasePreviewTest, DefaultStatusBarWidgetIsNull)
{
    TestAbstractBasePreview preview;
    EXPECT_EQ(preview.statusBarWidget(), nullptr);
}

TEST(AbstractBasePreviewTest, DefaultStatusBarAlignmentIsCenter)
{
    TestAbstractBasePreview preview;
    EXPECT_EQ(preview.statusBarWidgetAlignment(), Qt::AlignCenter);
}

TEST(AbstractBasePreviewTest, DefaultTitleIsEmpty)
{
    TestAbstractBasePreview preview;
    EXPECT_TRUE(preview.title().isEmpty());
}

TEST(AbstractBasePreviewTest, DefaultShowStatusBarSeparatorIsFalse)
{
    TestAbstractBasePreview preview;
    EXPECT_FALSE(preview.showStatusBarSeparator());
}

TEST(AbstractBasePreviewTest, PlayPauseStopAreNoop)
{
    TestAbstractBasePreview preview;
    EXPECT_NO_FATAL_FAILURE({ preview.play(); });
    EXPECT_NO_FATAL_FAILURE({ preview.pause(); });
    EXPECT_NO_FATAL_FAILURE({ preview.stop(); });
}

TEST(AbstractBasePreviewTest, InitializeAndHandleBeforeDestroyAreNoop)
{
    TestAbstractBasePreview preview;
    QWidget win;
    QWidget bar;
    EXPECT_NO_FATAL_FAILURE({ preview.initialize(&win, &bar); });
    EXPECT_NO_FATAL_FAILURE({ preview.handleBeforDestroy(); });
}

TEST(AbstractBasePreviewTest, SetFileUrlRoundTrips)
{
    TestAbstractBasePreview preview;
    QUrl url("file:///tmp/test.txt");
    preview.setFileUrl(url);
    EXPECT_EQ(preview.fileUrl(), url);
}

TEST(AbstractBasePreviewTest, LocalPreviewDestructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ TestAbstractBasePreview preview; });
}
// ============================================================
// PMS sev-2 regression cluster: abstractbasepreview.cpp (work-order batch 3)
// ============================================================

// PMS:236235 预览压缩包后关闭窗口崩溃：handleBeforDestroy 必须可重入/幂等，
// 有预览状态时连续两次调用不崩溃、不重复释放
TEST(AbstractBasePreviewTest, BUG236235_HandleBeforDestroyIdempotentNoop)
{
    TestAbstractBasePreview preview;
    preview.setFileUrl(QUrl::fromLocalFile("/tmp/dfm_ut_preview_236235.zip"));
    EXPECT_NO_FATAL_FAILURE({
        preview.handleBeforDestroy();
        preview.handleBeforDestroy();   // second invocation must stay a safe noop
    });
    EXPECT_EQ(preview.fileUrl(), QUrl::fromLocalFile("/tmp/dfm_ut_preview_236235.zip"));
}
