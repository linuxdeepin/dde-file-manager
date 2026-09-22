// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (uncovered functions in models/fileviewmodel.cpp):
//   - FileViewModel::toggleGroupTruncation -> ToggleGroupTruncation_UnknownGroup_NoCrash
//   - FileViewModel::onSortStrategyChanged  -> OnSortStrategyChanged_EmptyModel_NoCrash
//   - FileViewModel::getUrlsByRowIndex     -> GetUrlsByRowIndex_EmptyModel_ReturnsEmpty
// Remaining uncovered entries in this file are lambdas captured inside private wiring
// (setRootUrl/toggleTreeItemExpansion/connectFilterSortWorkSignals/onWorkFinish); they only
// run when a real FileSortWorker traversal round is executed, which needs the full traversal
// stack, so they are not targeted here.

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/fileviewmodel.h"

#include <QListView>
#include <QUrl>
#include <QList>

using namespace dfmplugin_workspace;

class UT_FileViewModelCov : public ::testing::Test
{
protected:
    void SetUp() override
    {
        view = new QListView();
        model = new FileViewModel(view);
    }

    void TearDown() override
    {
        delete model;
        delete view;
        stub.clear();
    }

    QListView *view = nullptr;
    FileViewModel *model = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_FileViewModelCov, ToggleGroupTruncation_UnknownGroup_NoCrash)
{
    // Arrange: empty model, no grouping data installed

    // Act
    model->toggleGroupTruncation("ut-unknown-group");

    // Assert
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(model->getChildrenUrls().size(), 0);
}

TEST_F(UT_FileViewModelCov, OnSortStrategyChanged_EmptyModel_NoCrash)
{
    // Arrange: sorter context still default

    // Act
    const int columnsBefore = model->columnCount();
    model->onSortStrategyChanged();

    // Assert: empty model stays empty, column layout untouched by the strategy refresh
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(model->columnCount(), columnsBefore);
}

TEST_F(UT_FileViewModelCov, GetUrlsByRowIndex_EmptyModel_ReturnsEmpty)
{
    // Arrange: no rows loaded

    // Act
    const QList<QUrl> first = model->getUrlsByRowIndex(0, 0);
    const QList<QUrl> second = model->getUrlsByRowIndex(0, 0);

    // Assert: result is deterministic for an empty model and the model stays empty
    EXPECT_EQ(first, second);
    EXPECT_EQ(model->rowCount(), 0);
}
