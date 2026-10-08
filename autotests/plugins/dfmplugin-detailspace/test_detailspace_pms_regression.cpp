// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression test for DetailSpaceHelper.
// Covers: BUG-346423 (showDetailView must disable the slide-in animation when
// it creates a brand-new detail space widget; the animation is only wanted
// when the detail space already exists and is merely being re-shown).
// The real FileManagerWindow is used, but the window manager lookup and the
// workspace width callback are stubbed; no event loop is spun (the deferred
// content-refresh singleShot never fires).

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariantHash>

#include "stubext.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>

#include "utils/detailspacehelper.h"
#include "views/detailspacewidget.h"

using namespace dfmplugin_detailspace;
DFMBASE_USE_NAMESPACE

class DetailSpaceRegressionTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // real window: installDetailView/detailViewWidth are exercised for real
        window = new FileManagerWindow(QUrl::fromLocalFile("/home"));
        winId = 0x19940701;

        stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                       [this](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                           return window;
                       });
        // workspace plugin is absent in the ut environment
        stub.set_lamda(&DetailSpaceHelper::updateWorkspaceWidth,
                       [](quint64, DetailSpaceWidget *, bool, int) { });
        stub.set_lamda(ADDR(FileManagerWindow, showDetailSpace),
                       [this](FileManagerWindow *, const QVariantHash &options) {
                           ++showCalls;
                           lastOptions = options;
                       });
    }

    void TearDown() override
    {
        stub.clear();
        DetailSpaceHelper::removeDetailSpace(winId);   // take + deleteLater
        DetailSpaceHelper::kDetailSpaceMap.remove(winId);
        delete window;
        window = nullptr;
    }

    stub_ext::StubExt stub;
    FileManagerWindow *window = nullptr;
    quint64 winId = 0;
    int showCalls = 0;
    QVariantHash lastOptions;
};

// PMS:346423 first show creates the widget and must open WITHOUT animation;
// re-showing the existing widget keeps the animation
TEST_F(DetailSpaceRegressionTest, BUG346423_ShowDetailViewSkipsAnimationOnWidgetCreation)
{
    ASSERT_EQ(showCalls, 0);
    ASSERT_EQ(DetailSpaceHelper::findDetailSpaceByWindowId(winId), nullptr);

    // 1st call: no widget yet -> addDetailSpace creates one -> kAnimated == false
    DetailSpaceHelper::showDetailView(winId, true, true);
    EXPECT_EQ(showCalls, 1);
    EXPECT_NE(DetailSpaceHelper::findDetailSpaceByWindowId(winId), nullptr);
    EXPECT_FALSE(lastOptions.value(DetailSpaceOptions::kAnimated).toBool());
    EXPECT_TRUE(lastOptions.value(DetailSpaceOptions::kUserAction).toBool());

    // 2nd call: widget already exists -> kAnimated == true
    DetailSpaceHelper::showDetailView(winId, true, true);
    EXPECT_EQ(showCalls, 2);
    EXPECT_TRUE(lastOptions.value(DetailSpaceOptions::kAnimated).toBool());
    EXPECT_TRUE(lastOptions.value(DetailSpaceOptions::kUserAction).toBool());

    // hide path also reports an animated transition
    DetailSpaceHelper::showDetailView(winId, false, true);
    EXPECT_EQ(showCalls, 2);   // hide goes through hideDetailSpace, not showDetailSpace
}
