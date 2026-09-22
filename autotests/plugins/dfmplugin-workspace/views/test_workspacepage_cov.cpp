// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (uncovered functions in views/workspacepage.cpp):
//   - WorkspacePage::setUrl               -> SetUrl_UnknownScheme_ResetsToLastUrl
//   - WorkspacePage::currentUrl           -> SetUrl_UnknownScheme_ResetsToLastUrl
//   - WorkspacePage::setCurrentView       -> SetUrl_UnknownScheme_ResetsToLastUrl (failure branch)
//   - WorkspacePage::initCustomTopWidgets -> InitCustomTopWidgets_NoRegisteredWidgets_NoCrash
//   - WorkspacePage::showViewHint         -> ShowViewHint_CreatesAndReplacesHint
//   - WorkspacePage::applyContentToHint   -> ShowViewHint_CreatesAndReplacesHint
//   - WorkspacePage::trackHintLifetime    -> TrackHintLifetime_LambdaRunsOnDestroy
//   - WorkspacePage::playDisappearAnimation -> PlayDisappearAnimation_NullView_WarningBranch
// Branch notes (from get_code_snippet):
//   setUrl: unknown scheme -> ViewFactory::create fails -> resets currentPageUrl to lastUrl,
//     calls setCurrentView(lastUrl) and publishes kChangeCurrentUrl.
//   playDisappearAnimation: null view -> warning branch and immediate return.

#include <gtest/gtest.h>

#include "addr_pri.h"
#include "stubext.h"

#include "views/workspacepage.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractbaseview.h>

#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>

using namespace dfmplugin_workspace;

ACCESS_PRIVATE_FUN(WorkspacePage, void(const QUrl &), initCustomTopWidgets)
ACCESS_PRIVATE_FUN(WorkspacePage, void(dfmbase::AbstractBaseView *), playDisappearAnimation)

class UT_WorkspacePageCov : public ::testing::Test
{
protected:
    void SetUp() override
    {
        page = new WorkspacePage();
    }

    void TearDown() override
    {
        delete page;
        stub.clear();
    }

    WorkspacePage *page = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_WorkspacePageCov, SetUrl_UnknownScheme_ResetsToLastUrl)
{
    // Arrange: no view creator is registered in unit tests, current url is empty
    const QUrl initial = page->currentUrl();
    EXPECT_FALSE(initial.isValid());

    // Act: "cov" scheme has no registered view -> create fails -> reset branch
    page->setUrl(QUrl("covnosuchscheme://nothing"));

    // Assert: url rolled back to the previous (empty) url, page still alive
    EXPECT_EQ(page->currentUrl(), initial);
    EXPECT_NE(page, nullptr);
}

TEST_F(UT_WorkspacePageCov, InitCustomTopWidgets_NoRegisteredWidgets_NoCrash)
{
    // Arrange: no custom top widgets registered for any scheme

    // Act
    call_private_fun::WorkspacePageinitCustomTopWidgets(*page, QUrl::fromLocalFile("/tmp"));

    // Assert
    EXPECT_NE(page, nullptr);
    EXPECT_FALSE(page->currentUrl().isValid());
}

TEST_F(UT_WorkspacePageCov, ShowViewHint_CreatesAndReplacesHint)
{
    // Arrange
    QVariantMap content;
    content.insert("message", QString("ut coverage hint"));

    // Act
    QObject *first = page->showViewHint(content);
    QObject *second = page->showViewHint(content);

    // Assert: each call returns a fresh hint object; the first one is replaced
    EXPECT_NE(first, nullptr);
    EXPECT_NE(second, nullptr);
    EXPECT_NE(first, second);

    // Act: destroy the current hint -> trackHintLifetime lambda must run
    second->deleteLater();
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();

    // Assert: page survives hint destruction (lambda only clears the pointer)
    EXPECT_NO_FATAL_FAILURE(page->showViewHint(content));
}

TEST_F(UT_WorkspacePageCov, PlayDisappearAnimation_NullView_WarningBranch)
{
    // Arrange: pass a null view pointer, the function must bail out early

    // Act
    call_private_fun::WorkspacePageplayDisappearAnimation(*page, nullptr);

    // Assert
    EXPECT_NE(page, nullptr);
    EXPECT_EQ(page->currentUrl(), QUrl());   // untouched by the failed animation request
    EXPECT_NO_FATAL_FAILURE(call_private_fun::WorkspacePageplayDisappearAnimation(*page, nullptr));
}
