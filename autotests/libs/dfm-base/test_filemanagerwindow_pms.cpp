// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindow_pms.cpp
 * @brief PMS sev-2 regression tests for FileManagerWindow
 *        (src/dfm-base/widgets/dfmwindow/filemanagerwindow.cpp).
 *
 * Bug -> case mapping (fix commit verified via `git show`):
 *   - PMS:312107 (81287c6d) FileManagerWindowPrivate::saveWindowState
 *     dereferenced the sidebar splitter without a null guard; closing a
 *     window that never installed the sidebar crashed. The fix adds
 *     `if (!splitter || splitter->sizes().isEmpty()) return;`.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QUrl>

#include "stubext.h"

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/dfmwindow/private/filemanagerwindow_p.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

using namespace dfmbase;

namespace {
class MockFrame : public AbstractFrame
{
public:
    explicit MockFrame(QWidget *parent = nullptr)
        : AbstractFrame(parent) {}
    void setCurrentUrl(const QUrl &url) override { url_ = url; }
    QUrl currentUrl() const override { return url_; }
    QUrl url_;
};
}   // namespace

// PMS:312107 未安装侧边栏(splitter 为空)时关闭窗口崩溃：
// saveWindowState 必须在 splitter 为空/无有效尺寸时直接返回
TEST(FileManagerWindowPmsTest, BUG312107_SaveWindowStateWithoutSplitterNoCrash)
{
    // Arrange — a bare window: no sidebar/workspace installed, so the
    // private splitter is null. Force a native window handle first, the
    // non-wayland branch reads windowHandle() properties before the
    // splitter guard.
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    (void)w.winId();

    const QVariantMap before = Application::appObtuselySetting()
                                       ->value("WindowManager", "WindowState")
                                       .toMap();

    // Act / Assert — the null-splitter early return must keep this crash-free.
    EXPECT_NO_FATAL_FAILURE({
        w.d.data()->saveWindowState();
    });

    // The early return must not have written a new state.
    const QVariantMap after = Application::appObtuselySetting()
                                      ->value("WindowManager", "WindowState")
                                      .toMap();
    EXPECT_EQ(before, after);
}

// PMS:312107 修复不得误伤：安装了侧边栏的正常路径仍要持久化窗口尺寸
TEST(FileManagerWindowPmsTest, BUG312107_SaveWindowStateWithSplitterPersistsSize)
{
    // Arrange — install the sidebar frame so the splitter exists.
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    w.installSideBar(new (std::nothrow) MockFrame());
    w.installWorkSpace(new (std::nothrow) MockFrame());
    (void)w.winId();
    w.resize(1200, 800);

    // Act
    EXPECT_NO_FATAL_FAILURE({
        w.d.data()->saveWindowState();
    });

    // Assert — width/height of the current window were persisted.
    const QVariantMap state = Application::appObtuselySetting()
                                      ->value("WindowManager", "WindowState")
                                      .toMap();
    EXPECT_GT(state.value("width").toInt(), 0);
    EXPECT_GT(state.value("height").toInt(), 0);
}
