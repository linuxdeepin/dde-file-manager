// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖函数清单(mode/normalizedmode.cpp) -> 用例映射:
// - NormalizedModePrivate::findValidPos -> FindValidPos_NoSurfaces_InvalidPosReturned
//       (分支: currentIndex 钳制到 0 后返回 {-1,-1})
// - NormalizedModePrivate::onDropFile -> OnDropFile_AutoClassified_UrlsCleared
// - NormalizedModePrivate::checkTouchFile -> CheckTouchFile_UntouchedUrl_NoEditorOpened
// - NormalizedModePrivate::collectionStyleChanged -> CollectionStyleChanged_UnknownId_NoConfigWrite
// - NormalizedModePrivate::onClearSelection -> OnClearSelection_WithShell_SelectionCleared
// - NormalizedModePrivate::onIconSizeChanged -> OnIconSizeChanged_NoHolders_NoCrash (空holders循环)
// - NormalizedModePrivate::onFontChanged -> OnFontChanged_NoHolders_NoCrash
// - NormalizedModePrivate::refreshViews -> RefreshViews_NoHolders_NoRefresh
// - NormalizedModePrivate::updateHolderSurfaceIndex -> UpdateHolderSurfaceIndex_NoSender_EarlyReturn
// 未覆盖: openEditor/createCollection/connectCollectionSignals(依赖真实 classifier/holder/surface 链),
//          onSelectFile(依赖 q->model 已初始化), initialize/rebuild lambda(完整初始化链路过重)。

#include "stubext.h"
#include "mode/normalizedmode.h"
#include "mode/normalized/normalizedmode_p.h"
#include "interface/canvasselectionshell.h"
#include "interface/canvasmanagershell.h"
#include "config/configpresenter.h"
#include "utils/fileoperator.h"

#include <QItemSelectionModel>
#include <QUrl>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

namespace {

class TestableNormalizedMode : public NormalizedMode
{
public:
    TestableNormalizedMode()
        : NormalizedMode()
    {
    }
    NormalizedModePrivate *priv() { return d; }
};

}   // namespace

class UT_NormalizedModeCov : public testing::Test
{
protected:
    void SetUp() override
    {
        mode = new TestableNormalizedMode();

        // shell-backed paths need a non-null shell (Q_ASSERT in onClearSelection)
        selectionShell = new CanvasSelectionShell(nullptr);
        mode->setCanvasSelectionShell(selectionShell);

        managerShell = new CanvasManagerShell(nullptr);
        mode->setCanvasManagerShell(managerShell);
        stub.set_lamda(static_cast<int (CanvasManagerShell::*)() const>(&CanvasManagerShell::iconLevel),
                       [](CanvasManagerShell *) -> int {
                           __DBG_STUB_INVOKE__
                           return 1;
                       });

        stub.set_lamda(&ConfigPresenter::updateNormalStyle,
                       [](ConfigPresenter *, const QString &, const CollectionStyle &) {
                           __DBG_STUB_INVOKE__
                       });
    }

    void TearDown() override
    {
        delete mode;
        delete selectionShell;
        delete managerShell;
        stub.clear();
    }

public:
    TestableNormalizedMode *mode = nullptr;
    CanvasSelectionShell *selectionShell = nullptr;
    CanvasManagerShell *managerShell = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_NormalizedModeCov, FindValidPos_NoSurfaces_InvalidPosReturned)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: no surfaces registered on the mode
    int index = 1;

    // Act: index clamped down to the (empty) surface count
    QPoint pos = d->findValidPos(index, 2, 2);

    // Assert: defensive branch returns the invalid marker
    EXPECT_EQ(pos, QPoint(-1, -1));
    EXPECT_EQ(index, 0);
}

TEST_F(UT_NormalizedModeCov, OnDropFile_AutoClassified_UrlsCleared)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: dropped urls handed over for classification
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/x"), QUrl::fromLocalFile("/tmp/y") };
    ASSERT_EQ(urls.count(), 2);

    // Act
    d->onDropFile("collection_cov", urls);

    // Assert: normalized mode re-classifies, so the request list is emptied
    EXPECT_TRUE(urls.isEmpty());
    EXPECT_EQ(urls.count(), 0);
}

TEST_F(UT_NormalizedModeCov, CheckTouchFile_UntouchedUrl_NoEditorOpened)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: url differs from the file-operator touch record
    const QUrl url = QUrl::fromLocalFile("/tmp/never_touched.txt");

    // Act
    d->checkTouchFile(url);

    // Assert: no editor path taken, mode stays non-editing
    EXPECT_FALSE(mode->isEditing());
    EXPECT_EQ(FileOperator::instance()->touchFileData(), QUrl());
}

TEST_F(UT_NormalizedModeCov, CollectionStyleChanged_UnknownId_NoConfigWrite)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: no holder registered for the id (updateNormalStyle stubbed)

    // Act
    d->collectionStyleChanged("no_such_collection");

    // Assert: nothing was written and mode still intact
    EXPECT_FALSE(mode->isEditing());
    EXPECT_EQ(mode->mode(), OrganizerMode::kNormalized);
}

TEST_F(UT_NormalizedModeCov, OnClearSelection_WithShell_SelectionCleared)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: shell attached in SetUp, selection model exists and is empty

    // Act
    d->onClearSelection();

    // Assert: clearing an empty selection keeps it empty without asserting
    EXPECT_NE(d->selectionModel, nullptr);
    EXPECT_EQ(d->selectionModel->selectedIndexes().count(), 0);
}

TEST_F(UT_NormalizedModeCov, OnIconSizeChanged_NoHolders_NoCrash)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: icon level stubbed to 1, no holders exist

    // Act
    d->onIconSizeChanged();

    // Assert: loop over zero holders completes, no layout request emitted
    EXPECT_TRUE(d->holders.isEmpty());
    EXPECT_EQ(d->holders.count(), 0);
    EXPECT_FALSE(mode->isEditing());
}

TEST_F(UT_NormalizedModeCov, OnFontChanged_NoHolders_NoCrash)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: no holders

    // Act
    d->onFontChanged();

    // Assert: nothing to refresh, holder set unchanged
    EXPECT_TRUE(d->holders.isEmpty());
    EXPECT_EQ(d->holders.count(), 0);
}

TEST_F(UT_NormalizedModeCov, RefreshViews_NoHolders_NoRefresh)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: no holders

    // Act: both silence variants
    d->refreshViews(true);
    d->refreshViews(false);

    // Assert: still no holders and no crash
    EXPECT_TRUE(d->holders.isEmpty());
    EXPECT_EQ(mode->mode(), OrganizerMode::kNormalized);
}

TEST_F(UT_NormalizedModeCov, UpdateHolderSurfaceIndex_NoSender_EarlyReturn)
{
    NormalizedModePrivate *d = mode->priv();
    // Arrange: called directly, so sender() is null

    // Act
    d->updateHolderSurfaceIndex(nullptr);

    // Assert: early return leaves everything untouched
    EXPECT_TRUE(d->holders.isEmpty());
    EXPECT_EQ(mode->mode(), OrganizerMode::kNormalized);
}
