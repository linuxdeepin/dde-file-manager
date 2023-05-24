// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/canvasselectionmodel.h"
#include "model/canvasproxymodel_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE

TEST(CanvasSelectionModel, model)
{
    CanvasProxyModel model;
    CanvasSelectionModel sel(&model, nullptr);
    EXPECT_EQ(sel.model(), &model);
}

TEST(CanvasSelectionModel, selectedUrls)
{
    CanvasProxyModel model;
    QUrl url = QUrl::fromLocalFile("/home");
    model.d->fileList.append(url);
    model.d->fileMap.insert(url, {});

    CanvasSelectionModel sel(&model, nullptr);

    stub_ext::StubExt stub;
    QModelIndexList list;
    list.append(model.index(0));
    stub.set_lamda(&CanvasSelectionModel::selectedIndexes, [&model, &list](){
        return list;
    });

    auto ret = sel.selectedIndexesCache();
    ASSERT_EQ(ret.size(), 1);
    EXPECT_EQ(ret.first().row(), 0);

    auto urls = sel.selectedUrls();
    ASSERT_EQ(urls.size(), 1);
    EXPECT_EQ(urls.first(), url);

    list.append(QModelIndex());

    EXPECT_EQ(ret, sel.selectedIndexesCache());
}

TEST(CanvasSelectionModel, clearSelectedCache)
{
    CanvasProxyModel model;
    CanvasSelectionModel sel(&model, nullptr);
    sel.selectedCache.append(QModelIndex());

    sel.clearSelectedCache();
    EXPECT_TRUE(sel.selectedIndexesCache().isEmpty());
}

