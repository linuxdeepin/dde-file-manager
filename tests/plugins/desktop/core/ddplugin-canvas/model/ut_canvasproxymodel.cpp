// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "model/canvasproxymodel_p.h"
#include "model/fileinfomodel_p.h"

#include "stubext.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>

#include <QStandardPaths>
#include <QMimeData>
#include <QModelIndex>
#include <QUrl>

DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(CanvasProxyModel, construct)
{
    CanvasProxyModel model;
    EXPECT_EQ(model.d->isNotMixDirAndFile, !Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool());
}

TEST(CanvasProxyModel, setSourceModel)
{
    FileInfoModel fm;
    CanvasProxyModel model;
    model.d->fileList.append(QUrl());

    bool br = false;
    QObject::connect(&model, &CanvasProxyModel::modelAboutToBeReset, &model, [&br]() {
        br = true;
    });

    bool er = false;
    QObject::connect(&model, &CanvasProxyModel::modelReset, &model, [&er]() {
        er = true;
    });

    model.setSourceModel(&fm);
    ASSERT_EQ(model.d->srcModel, &fm);
    EXPECT_EQ(model.sourceModel(), &fm);
    EXPECT_TRUE(br);
    EXPECT_TRUE(er);
    EXPECT_TRUE(model.d->fileList.isEmpty());
    EXPECT_TRUE(model.d->fileMap.isEmpty());

    // check signal;
    stub_ext::StubExt stub;
    {
        QUrl oldurl;
        QUrl newurl;
        stub.set_lamda(&CanvasProxyModelPrivate::sourceDataRenamed, [&oldurl, &newurl](CanvasProxyModelPrivate *, const QUrl &t1, const QUrl &t2) {
            oldurl = t1;
            newurl = t2;
        });
        auto t1 = QUrl::fromLocalFile("/home");
        auto t2 = QUrl::fromLocalFile("/home2");
        emit fm.dataReplaced(t1, t2);
        EXPECT_EQ(oldurl, t1);
        EXPECT_EQ(newurl, t2);
    }

    {
        QModelIndex tl;
        QModelIndex br;
        stub.set_lamda(&CanvasProxyModelPrivate::sourceDataChanged, [&tl, &br](CanvasProxyModelPrivate *, const QModelIndex &sourceTopleft, const QModelIndex &sourceBottomright, const QVector<int> &roles) {
            tl = sourceTopleft;
            br = sourceBottomright;
        });
        QModelIndex t1(0, 0, nullptr, &model);
        QModelIndex t2(5, 0, nullptr, &model);
        emit fm.dataChanged(t1, t2);
        EXPECT_EQ(tl, t1);
        EXPECT_EQ(br, t2);
    }

    {
        int istart = -1;
        int iend = -1;
        stub.set_lamda(&CanvasProxyModelPrivate::sourceRowsInserted, [&istart, &iend](CanvasProxyModelPrivate *, const QModelIndex &sourceParent, int start, int end) {
            istart = start;
            iend = end;
        });
        emit fm.rowsInserted(QModelIndex(), 1, 3, {});
        EXPECT_EQ(istart, 1);
        EXPECT_EQ(iend, 3);
    }

    {
        int istart = -1;
        int iend = -1;
        stub.set_lamda(&CanvasProxyModelPrivate::sourceRowsAboutToBeRemoved, [&istart, &iend](CanvasProxyModelPrivate *, const QModelIndex &sourceParent, int start, int end) {
            istart = start;
            iend = end;
        });
        emit fm.rowsAboutToBeRemoved(QModelIndex(), 1, 3, {});
        EXPECT_EQ(istart, 1);
        EXPECT_EQ(iend, 3);
    }

    {
        bool be = false;
        stub.set_lamda(&CanvasProxyModelPrivate::sourceAboutToBeReset, [&be]() {
            be = true;
        });
        emit fm.modelAboutToBeReset({});
        EXPECT_TRUE(be);
    }

    {
        bool end = false;
        stub.set_lamda(&CanvasProxyModelPrivate::sourceReset, [&end]() {
            end = true;
        });
        emit fm.modelReset({});
        EXPECT_TRUE(end);
    }
}

TEST(CanvasProxyModel, rootIndex)
{
    CanvasProxyModel model;
    EXPECT_EQ(model.rootIndex().row(), INT_MAX);
}

TEST(CanvasProxyModel, rootUrl)
{
    CanvasProxyModel model;
    stub_ext::StubExt stub;
    QModelIndex index;
    stub.set_lamda(&CanvasProxyModel::fileUrl, [&index](CanvasProxyModel *m, const QModelIndex &idx) {
        index = idx;
        return QUrl::fromLocalFile("/home");
    });

    EXPECT_EQ(model.rootUrl(), QUrl::fromLocalFile("/home"));
    EXPECT_EQ(index, model.rootIndex());
}

TEST(CanvasProxyModel, files)
{
    CanvasProxyModel model;
    model.d->fileList.append(QUrl());
    model.d->fileList.append(QUrl());

    EXPECT_EQ(model.files(), model.d->fileList);
}

TEST(CanvasProxyModel, index)
{
    CanvasProxyModel model;
    QUrl url = QUrl::fromLocalFile("/home");
    model.d->fileList.append(url);
    model.d->fileMap.insert(url, {});

    EXPECT_FALSE(model.index(QUrl()).isValid());
    EXPECT_FALSE(model.index(QUrl::fromLocalFile("/home2")).isValid());
    QModelIndex ret = model.index(QUrl::fromLocalFile("/home"));
    ASSERT_TRUE(ret.isValid());
    EXPECT_EQ(ret.row(), 0);
}

TEST(CanvasProxyModel, fileUrl)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    stub_ext::StubExt stub;
    stub.set_lamda(&FileInfoModel::rootUrl, []() {
        return QUrl::fromLocalFile("/home");
    });
    model.d->srcModel = &fm;

    QUrl url = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(url);

    EXPECT_EQ(model.fileUrl(model.rootIndex()), QUrl::fromLocalFile("/home"));
    EXPECT_TRUE(model.fileUrl(QModelIndex()).isEmpty());
    EXPECT_TRUE(model.fileUrl(QModelIndex(1, 0, nullptr, &model)).isEmpty());
    EXPECT_EQ(model.fileUrl(QModelIndex(0, 0, nullptr, &model)), url);
}

TEST(CanvasProxyModel, fileInfo)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    stub_ext::StubExt stub;
    stub.set_lamda(&FileInfoModel::rootUrl, []() {
        return QUrl::fromLocalFile("/home");
    });
    model.d->srcModel = &fm;

    EXPECT_EQ(model.fileInfo(model.rootIndex())->urlOf(FileInfo::FileUrlInfoType::kUrl), QUrl::fromLocalFile("/home"));

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));

    model.d->fileMap.insert(in1, info1);
    model.d->fileMap.insert(in2, info2);
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);

    EXPECT_EQ(model.fileInfo(QModelIndex()), nullptr);
    EXPECT_EQ(model.fileInfo(QModelIndex(2, 0, nullptr, &model)), nullptr);

    EXPECT_EQ(model.fileInfo(QModelIndex(0, 0, nullptr, &model)), info1);
    EXPECT_EQ(model.fileInfo(QModelIndex(1, 0, nullptr, &model)), info2);
}

TEST(CanvasProxyModel, rowCount)
{
    CanvasProxyModel model;
    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);
    EXPECT_EQ(model.rowCount(QModelIndex()), 0);
    EXPECT_EQ(model.rowCount(QModelIndex(0, 0, nullptr, &model)), 0);
    EXPECT_EQ(model.rowCount(model.rootIndex()), 2);
}

TEST(CanvasProxyModel, columnCount)
{
    CanvasProxyModel model;
    EXPECT_EQ(model.columnCount(QModelIndex()), 0);
    EXPECT_EQ(model.columnCount(QModelIndex(0, 0, nullptr, &model)), 0);
    EXPECT_EQ(model.columnCount(model.rootIndex()), 1);
}

TEST(CanvasProxyModel, data_and_map)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    model.d->srcModel = &fm;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(QUrl::fromLocalFile("/home/test3"));   // for mapToSource
    model.d->fileList.append(in1);

    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);

    stub_ext::StubExt stub;
    QModelIndex index;
    int itemRole = -1;
    stub.set_lamda(VADDR(FileInfoModel, data), [&index, &itemRole](FileInfoModel *, const QModelIndex &idx, int role) {
        index = idx;
        itemRole = role;
        return QVariant("test");
    });

    {
        QVariant ret = model.data(QModelIndex(), 2);
        EXPECT_FALSE(ret.isValid());
        EXPECT_FALSE(index.isValid());
        EXPECT_EQ(itemRole, -1);
    }

    {
        itemRole = -1;
        index = QModelIndex();
        QVariant ret = model.data(QModelIndex(0, 0, nullptr, &model), 2);
        EXPECT_FALSE(ret.isValid());
        EXPECT_FALSE(index.isValid());
        EXPECT_EQ(itemRole, -1);
    }

    {
        itemRole = -1;
        index = QModelIndex();
        QVariant ret = model.data(QModelIndex(2, 0, nullptr, &model), 2);
        EXPECT_FALSE(ret.isValid());
        EXPECT_FALSE(index.isValid());
        EXPECT_EQ(itemRole, -1);
    }

    {
        itemRole = -1;
        index = QModelIndex();
        QVariant ret = model.data(QModelIndex(1, 0, nullptr, &model), 2);
        EXPECT_EQ(ret.toString(), QString("test"));
        EXPECT_TRUE(index.isValid());
        EXPECT_EQ(itemRole, 2);
        EXPECT_EQ(index.row(), 0);
    }
}

TEST(CanvasProxyModel, mimeData)
{
    CanvasProxyModel model;
    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);

    auto mm = model.mimeData({ QModelIndex(0, 0, nullptr, &model), QModelIndex(1, 0, nullptr, &model) });
    ASSERT_NE(mm, nullptr);
    auto ret = mm->urls();
    ASSERT_EQ(ret.size(), 2);
    EXPECT_EQ(ret.first(), in1);
    EXPECT_EQ(ret.last(), in2);

    EXPECT_EQ(mm->data("userid_for_drag"), QString::number(getuid()));
    delete mm;
}

TEST(CanvasProxyModel, sort)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    model.d->srcModel = &fm;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    model.d->fileMap.insert(in1, info1);
    model.d->fileMap.insert(in2, info2);
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);

    QPersistentModelIndex idx1 = model.index(in1);
    QPersistentModelIndex idx2 = model.index(in2);

    bool lac = false;
    QObject::connect(&model, &CanvasProxyModel::layoutAboutToBeChanged, &model, [&lac]() {
        lac = true;
    });

    bool lc = false;
    QObject::connect(&model, &CanvasProxyModel::layoutChanged, &model, [&lc]() {
        lc = true;
    });

    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasProxyModelPrivate::doSort, [](CanvasProxyModelPrivate *, QList<QUrl> &files) {
        EXPECT_EQ(files.size(), 2);
        files.move(0, 1);
        return true;
    });

    model.sort();

    EXPECT_TRUE(lac);
    EXPECT_TRUE(lc);

    ASSERT_EQ(model.d->fileList.size(), 2);
    EXPECT_EQ(model.d->fileList.first(), in2);
    EXPECT_EQ(model.d->fileList.last(), in1);
    EXPECT_EQ(idx1.row(), 1);
    EXPECT_EQ(idx2.row(), 0);
}

TEST(CanvasProxyModelPrivate, indexs)
{
    CanvasProxyModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);
    model.d->fileMap.insert(in1, {});
    model.d->fileMap.insert(in2, {});

    auto idxs = model.d->indexs();
    ASSERT_EQ(idxs.size(), 2);
    EXPECT_EQ(idxs.first().row(), 0);
    EXPECT_EQ(idxs.last().row(), 1);

    idxs = model.d->indexs({ QUrl(), in1, QUrl::fromLocalFile("/home/test3"), in2 });
    ASSERT_EQ(idxs.size(), 4);
    EXPECT_FALSE(idxs.first().isValid());
    EXPECT_TRUE(idxs.at(1).isValid());
    EXPECT_FALSE(idxs.at(2).isValid());
    EXPECT_TRUE(idxs.at(3).isValid());
    EXPECT_EQ(idxs.at(1).row(), 0);
    EXPECT_EQ(idxs.at(3).row(), 1);
}

TEST(CanvasProxyModel, fetch)
{
    CanvasProxyModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));

    model.d->fileList.append(in1);
    model.d->fileMap.insert(in1, info1);

    FileInfoModel fm;
    model.d->srcModel = &fm;
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);

    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);

    int bi = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsAboutToBeInserted, &model, [&bi, &model](const QModelIndex &parent, int first, int last) {
        bi = first;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, last);
    });
    int ei = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsInserted, &model, [&ei, &model](const QModelIndex &parent, int first, int last) {
        ei = first;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, last);
    });

    bool filter = true;
    QUrl filterUrl;
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasProxyModelPrivate::insertFilter, [&filter, &filterUrl](CanvasProxyModelPrivate *, const QUrl &url) {
        filterUrl = url;
        return filter;
    });

    {
        EXPECT_TRUE(model.fetch(in1));
        EXPECT_TRUE(filterUrl.isEmpty());
        EXPECT_EQ(bi, -1);
        EXPECT_EQ(ei, -1);
        EXPECT_EQ(model.d->fileList.size(), 1);
    }

    {
        filterUrl.clear();
        ei = -1;
        bi = -1;
        EXPECT_FALSE(model.fetch(QUrl::fromLocalFile("/home/test3")));
        EXPECT_TRUE(filterUrl.isEmpty());
        EXPECT_EQ(bi, -1);
        EXPECT_EQ(ei, -1);
        EXPECT_EQ(model.d->fileList.size(), 1);
    }

    {
        filterUrl.clear();
        ei = -1;
        bi = -1;
        EXPECT_FALSE(model.fetch(in2));
        EXPECT_EQ(filterUrl, in2);
        EXPECT_EQ(bi, -1);
        EXPECT_EQ(ei, -1);
        EXPECT_EQ(model.d->fileList.size(), 1);
    }

    {
        filterUrl.clear();
        ei = -1;
        bi = -1;
        filter = false;
        EXPECT_TRUE(model.fetch(in2));
        EXPECT_EQ(filterUrl, in2);
        EXPECT_EQ(bi, 1);
        EXPECT_EQ(ei, 1);
        EXPECT_EQ(model.d->fileList.size(), 2);
        EXPECT_EQ(model.d->fileMap.value(in2), info2);
    }
}

TEST(CanvasProxyModel, take)
{
    CanvasProxyModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));

    model.d->fileList.append(in1);
    model.d->fileMap.insert(in1, info1);

    int br = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsAboutToBeRemoved, &model, [&br, &model](const QModelIndex &parent, int first, int last) {
        br = first;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, last);
    });
    int er = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsRemoved, &model, [&er, &model](const QModelIndex &parent, int first, int last) {
        er = first;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, last);
    });

    QUrl filterUrl;
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasProxyModelPrivate::removeFilter, [&filterUrl](CanvasProxyModelPrivate *, const QUrl &url) {
        filterUrl = url;
        return false;
    });

    {
        EXPECT_TRUE(model.take(QUrl::fromLocalFile("/home/test2")));
        EXPECT_TRUE(filterUrl.isEmpty());
        EXPECT_EQ(br, -1);
        EXPECT_EQ(er, -1);
        EXPECT_EQ(model.d->fileList.size(), 1);
    }

    {
        filterUrl.clear();
        br = -1;
        er = -1;
        EXPECT_TRUE(model.take(in1));
        EXPECT_EQ(filterUrl, in1);
        EXPECT_EQ(br, 0);
        EXPECT_EQ(er, 0);
        EXPECT_TRUE(model.d->fileList.isEmpty());
        EXPECT_TRUE(model.d->fileMap.isEmpty());
    }
}

TEST(CanvasProxyModel, refresh)
{
    CanvasProxyModel model;
    model.refresh(model.rootIndex());
    ASSERT_NE(model.d->refreshTimer, nullptr);
    EXPECT_TRUE(model.d->refreshTimer->isActive());
    EXPECT_TRUE(model.d->refreshTimer->isSingleShot());
}

TEST(CanvasProxyModelPrivate, createMapping)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    model.d->srcModel = &fm;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    auto in3 = QUrl::fromLocalFile("/home/test3");
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);
    fm.d->fileList.append(in3);
    model.d->fileList.append(in3);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    DFMSyncFileInfoPointer info3(new SyncFileInfo(in3));
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);
    fm.d->fileMap.insert(in3, info3);
    model.d->fileMap.insert(in3, info3);

    stub_ext::StubExt stub;
    QList<QUrl> filterUrl;
    stub.set_lamda(&CanvasProxyModelPrivate::resetFilter, [&filterUrl](CanvasProxyModelPrivate *, QList<QUrl> &url) {
        filterUrl = url;
        EXPECT_EQ(url.size(), 3);
        url.removeAt(1);
        return true;
    });

    bool sort = false;
    stub.set_lamda(&CanvasProxyModelPrivate::doSort, [&sort](CanvasProxyModelPrivate *, QList<QUrl> &url) {
        EXPECT_EQ(url.size(), 2);
        sort = true;
        url.move(0, 1);
        return true;
    });

    model.d->createMapping();
    EXPECT_EQ(filterUrl, fm.d->fileList);
    EXPECT_TRUE(sort);
    ASSERT_EQ(model.d->fileList.size(), 2);
    EXPECT_EQ(model.d->fileList.at(0), in3);
    EXPECT_EQ(model.d->fileList.at(1), in1);
    EXPECT_EQ(model.d->fileMap.value(in1), info1);
    EXPECT_EQ(model.d->fileMap.value(in3), info3);
    EXPECT_FALSE(model.d->fileMap.contains(in2));
}

TEST(CanvasProxyModelPrivate, sourceAboutToBeReset)
{
    CanvasProxyModel model;
    bool reset = false;
    QObject::connect(&model, &CanvasProxyModel::modelAboutToBeReset, &model, [&reset]() {
        reset = true;
    });

    model.d->sourceAboutToBeReset();
    EXPECT_TRUE(reset);
}

TEST(CanvasProxyModelPrivate, sourceReset)
{
    CanvasProxyModel model;
    bool reset = false;
    QObject::connect(&model, &CanvasProxyModel::modelReset, &model, [&reset]() {
        reset = true;
    });

    stub_ext::StubExt stub;
    bool map = false;
    stub.set_lamda(&CanvasProxyModelPrivate::createMapping, [&map]() {
        map = true;
    });

    model.d->sourceReset();
    EXPECT_TRUE(reset);
    EXPECT_TRUE(map);
}

TEST(CanvasProxyModelPrivate, sourceRowsInserted)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    model.d->srcModel = &fm;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    auto in3 = QUrl::fromLocalFile("/home/test3");
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);
    fm.d->fileList.append(in3);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    DFMSyncFileInfoPointer info3(new SyncFileInfo(in3));
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);
    fm.d->fileMap.insert(in3, info3);

    QUrl filterUrl;
    stub_ext::StubExt stub;
    bool filterIns = true;
    stub.set_lamda(&CanvasProxyModelPrivate::insertFilter, [&filterUrl, &filterIns](CanvasProxyModelPrivate *, const QUrl &url) {
        filterUrl = url;
        return filterIns;
    });

    int br = -1;
    int br2 = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsAboutToBeInserted, &model, [&br, &br2, &model](const QModelIndex &parent, int first, int last) {
        br = first;
        br2 = last;
        EXPECT_EQ(parent, model.rootIndex());
    });

    int er = -1;
    int er2 = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsInserted, &model, [&er, &er2, &model](const QModelIndex &parent, int first, int last) {
        er = first;
        er2 = last;
        EXPECT_EQ(parent, model.rootIndex());
    });

    {
        model.d->sourceRowsInserted(QModelIndex(), 1, 1);
        EXPECT_EQ(filterUrl, in2);
        EXPECT_EQ(br, -1);
        EXPECT_EQ(br2, -1);
        EXPECT_EQ(er, -1);
        EXPECT_EQ(er2, -1);
        EXPECT_TRUE(model.d->fileList.isEmpty());
    }

    {
        br = -1;
        br2 = -1;
        er = -1;
        er2 = -1;
        filterIns = false;

        model.d->sourceRowsInserted(QModelIndex(), 1, 2);
        EXPECT_EQ(br, 0);
        EXPECT_EQ(br2, 1);
        EXPECT_EQ(er, 0);
        EXPECT_EQ(er2, 1);
        ASSERT_EQ(model.d->fileList.size(), 2);
        EXPECT_EQ(model.d->fileMap.value(model.d->fileList.at(0)), info2);
        EXPECT_EQ(model.d->fileMap.value(model.d->fileList.at(1)), info3);
    }
}

TEST(CanvasProxyModelPrivate, sourceRowsAboutToBeRemoved)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    model.d->srcModel = &fm;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    auto in3 = QUrl::fromLocalFile("/home/test3");
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);
    fm.d->fileList.append(in3);
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);
    model.d->fileList.append(in3);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    DFMSyncFileInfoPointer info3(new SyncFileInfo(in3));
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);
    fm.d->fileMap.insert(in3, info3);
    model.d->fileMap.insert(in1, info1);
    model.d->fileMap.insert(in2, info2);
    model.d->fileMap.insert(in3, info3);

    QUrl filterUrl;
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasProxyModelPrivate::removeFilter, [&filterUrl](CanvasProxyModelPrivate *, const QUrl &url) {
        filterUrl = url;
        return true;
    });

    int br = -1;
    int br2 = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsAboutToBeRemoved, &model, [&br, &br2, &model](const QModelIndex &parent, int first, int last) {
        br = first;
        br2 = last;
        EXPECT_EQ(parent, model.rootIndex());
    });

    int er = -1;
    int er2 = -1;
    QObject::connect(&model, &CanvasProxyModel::rowsRemoved, &model, [&er, &er2, &model](const QModelIndex &parent, int first, int last) {
        er = first;
        er2 = last;
        EXPECT_EQ(parent, model.rootIndex());
    });

    {
        model.d->sourceRowsAboutToBeRemoved(QModelIndex(), 1, 1);
        EXPECT_EQ(filterUrl, in2);
        EXPECT_EQ(br, 1);
        EXPECT_EQ(br2, 1);
        EXPECT_EQ(er, 1);
        EXPECT_EQ(er2, 1);
        EXPECT_EQ(model.d->fileList.size(), 2);
        EXPECT_FALSE(model.d->fileMap.contains(in2));
    }

    {
        br = -1;
        br2 = -1;
        er = -1;
        er2 = -1;

        model.d->sourceRowsAboutToBeRemoved(QModelIndex(), 2, 2);
        EXPECT_EQ(br, 1);
        EXPECT_EQ(br2, 1);
        EXPECT_EQ(er, 1);
        EXPECT_EQ(er2, 1);
        ASSERT_EQ(model.d->fileList.size(), 1);
    }
}

TEST(CanvasProxyModelPrivate, sourceDataRenamed)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    model.d->srcModel = &fm;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);

    bool up = false;
    QObject::connect(&model, &CanvasProxyModel::dataChanged, &model,
                     [&up, &model](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                         up = true;
                         EXPECT_EQ(topLeft.row(), bottomRight.row());
                     });

    QUrl oldu;
    QUrl newu;
    QObject::connect(&model, &CanvasProxyModel::dataReplaced, &model,
                     [&oldu, &newu](const QUrl &t1, const QUrl &t2) {
                         oldu = t1;
                         newu = t2;
                     });

    model.d->fileList.append(in1);
    model.d->fileMap.insert(in1, info1);

    stub_ext::StubExt stub;
    bool filter = true;
    stub.set_lamda(&CanvasProxyModelPrivate::renameFilter, [&filter](CanvasProxyModelPrivate *, const QUrl &oldUrl, const QUrl &newUrl) {
        return filter;
    });

    // filter
    {
        model.d->sourceDataRenamed(in1, in2);
        EXPECT_FALSE(up);
        EXPECT_TRUE(model.d->fileList.isEmpty());
        EXPECT_TRUE(oldu.isEmpty());
        EXPECT_TRUE(newu.isEmpty());
    }

    // move a to b, a is not existed
    {
        model.d->fileList.clear();
        model.d->fileMap.clear();
        filter = false;

        up = false;
        oldu = QUrl();
        newu = QUrl();
        model.d->sourceDataRenamed(in1, in2);
        EXPECT_FALSE(up);
        EXPECT_EQ(model.d->fileList.size(), 1);
        EXPECT_EQ(model.d->fileMap.value(in2), info2);
        EXPECT_TRUE(oldu.isEmpty());
        EXPECT_TRUE(newu.isEmpty());
    }

    // move a to b, b is existed
    {
        model.d->fileList.clear();
        model.d->fileMap.clear();

        model.d->fileList.append(in1);
        model.d->fileMap.insert(in1, info1);
        model.d->fileList.append(in2);
        model.d->fileMap.insert(in2, info2);

        up = false;
        oldu = QUrl();
        newu = QUrl();
        model.d->sourceDataRenamed(in1, in2);

        EXPECT_EQ(model.d->fileList.size(), 1);
        EXPECT_EQ(model.d->fileMap.value(in2), info2);
        EXPECT_TRUE(oldu.isEmpty());
        EXPECT_TRUE(newu.isEmpty());
        EXPECT_TRUE(up);
    }

    // move a to b, b is not existed
    {
        model.d->fileList.clear();
        model.d->fileMap.clear();
        model.d->fileList.append(in1);
        model.d->fileMap.insert(in1, info1);

        up = false;
        oldu = QUrl();
        newu = QUrl();
        model.d->sourceDataRenamed(in1, in2);

        EXPECT_EQ(oldu, in1);
        EXPECT_EQ(newu, in2);
        EXPECT_TRUE(up);
        ASSERT_EQ(model.d->fileList.size(), 1);
        EXPECT_EQ(model.d->fileList.first(), in2);
        ASSERT_EQ(model.d->fileMap.size(), 1);
        EXPECT_TRUE(model.d->fileMap.contains(in2));
    }
}

TEST(CanvasProxyModelPrivate, lessThan)
{
    CanvasProxyModel model;
    FileInfoModel fm;
    model.d->srcModel = &fm;

    auto in1 = QUrl::fromLocalFile("/var");
    auto in2 = QUrl::fromLocalFile("/usr");
    fm.d->fileList.append(in1);
    fm.d->fileList.append(in2);
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    fm.d->fileMap.insert(in1, info1);
    fm.d->fileMap.insert(in2, info2);
    model.d->fileMap.insert(in1, info1);
    model.d->fileMap.insert(in2, info2);

    model.d->isNotMixDirAndFile = true;
    model.d->fileSortOrder = Qt::AscendingOrder;
    model.d->fileSortRole = Global::kItemFileDisplayNameRole;

    EXPECT_FALSE(model.d->lessThan(in1, in2));

    model.d->fileSortOrder = Qt::DescendingOrder;
    EXPECT_TRUE(model.d->lessThan(in1, in2));
}

TEST(CanvasProxyModelPrivate, doSort)
{
    CanvasProxyModel model;
    QUrl root = QUrl::fromLocalFile("/home/test");
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasProxyModel::rootUrl, [&root]() {
        return root;
    });
    stub.set_lamda(&CanvasProxyModelPrivate::lessThan, [](CanvasProxyModelPrivate *d, const QUrl &left, const QUrl &right) {
        return FileUtils::compareString(left.fileName(), right.fileName(), d->fileSortOrder);
    });

    QUrl computer = QUrl::fromLocalFile("/home/test/dde-computer.desktop");
    QUrl trash = QUrl::fromLocalFile("/home/test/dde-trash.desktop");
    QUrl home = QUrl::fromLocalFile("/home/test/dde-home.desktop");
    QUrl filea = QUrl::fromLocalFile("/home/test/a.txt");
    QUrl fileb = QUrl::fromLocalFile("/home/test/b.txt");
    QUrl filec = QUrl::fromLocalFile("/home/test/c.txt");

    const QList<QUrl> urls = { filea, trash, computer, home, fileb, filec };

    {
        auto check = urls;
        model.d->fileSortRole = Global::kItemNameRole;
        model.d->fileSortOrder = Qt::AscendingOrder;
        model.d->doSort(check);
        ASSERT_EQ(check.size(), 6);
        EXPECT_EQ(check.at(0), filea);
        EXPECT_EQ(check.at(1), fileb);
        EXPECT_EQ(check.at(2), filec);
        EXPECT_EQ(check.at(3), computer);
        EXPECT_EQ(check.at(4), home);
        EXPECT_EQ(check.at(5), trash);
    }

    {
        auto check = urls;
        model.d->fileSortRole = Global::kItemNameRole;
        model.d->fileSortOrder = Qt::DescendingOrder;
        model.d->doSort(check);
        ASSERT_EQ(check.size(), 6);
        EXPECT_EQ(check.at(5), filea);
        EXPECT_EQ(check.at(4), fileb);
        EXPECT_EQ(check.at(3), filec);
        EXPECT_EQ(check.at(2), computer);
        EXPECT_EQ(check.at(1), home);
        EXPECT_EQ(check.at(0), trash);
    }

    {
        auto check = urls;
        model.d->fileSortRole = Global::kItemFileMimeTypeRole;
        model.d->fileSortOrder = Qt::AscendingOrder;
        model.d->doSort(check);
        ASSERT_EQ(check.size(), 6);
        EXPECT_EQ(check.at(0), computer);
        EXPECT_EQ(check.at(1), trash);
        EXPECT_EQ(check.at(2), home);
        EXPECT_EQ(check.at(3), filea);
        EXPECT_EQ(check.at(4), fileb);
        EXPECT_EQ(check.at(5), filec);
    }

    {
        auto check = urls;
        model.d->fileSortRole = Global::kItemFileMimeTypeRole;
        model.d->fileSortOrder = Qt::DescendingOrder;
        model.d->doSort(check);
        ASSERT_EQ(check.size(), 6);
        EXPECT_EQ(check.at(5), computer);
        EXPECT_EQ(check.at(4), trash);
        EXPECT_EQ(check.at(3), home);
        EXPECT_EQ(check.at(2), filea);
        EXPECT_EQ(check.at(1), fileb);
        EXPECT_EQ(check.at(0), filec);
    }
}

class TCanvastModelFilter : public CanvasModelFilter
{
public:
    TCanvastModelFilter()
        : CanvasModelFilter(nullptr) {}
    bool resetFilter(QList<QUrl> &urls) override
    {
        ct = true;
        if (rt)
            urls.clear();
        return rt;
    }
    bool removeFilter(const QUrl &url) override
    {
        cd = true;
        return rd;
    }
    bool insertFilter(const QUrl &url) override
    {
        cc = true;
        return rc;
    }
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl) override
    {
        cr = true;
        return rr;
    }
    bool updateFilter(const QUrl &url, const QVector<int> &roles) override
    {
        cu = true;
        return ru;
    }
    bool ct = false;
    bool cd = false;
    bool cc = false;
    bool cr = false;
    bool cu = false;
    bool rt = false;
    bool rd = false;
    bool rc = false;
    bool rr = false;
    bool ru = false;
};

namespace {
class TestCanvasModelFilter : public testing::Test
{
public:
    void SetUp() override
    {
        QSharedPointer<CanvasModelFilter> filter1(new TCanvastModelFilter);
        f1 = dynamic_cast<TCanvastModelFilter *>(filter1.get());
        model.d->modelFilters.append(filter1);
        QSharedPointer<CanvasModelFilter> filter2(new TCanvastModelFilter);
        f2 = dynamic_cast<TCanvastModelFilter *>(filter2.get());
        model.d->modelFilters.append(filter2);
        QSharedPointer<CanvasModelFilter> filter3(new TCanvastModelFilter);
        f3 = dynamic_cast<TCanvastModelFilter *>(filter3.get());
        model.d->modelFilters.append(filter3);
    }
    CanvasProxyModel model;
    TCanvastModelFilter *f1;
    TCanvastModelFilter *f2;
    TCanvastModelFilter *f3;
};
}

TEST_F(TestCanvasModelFilter, insertFilter)
{
    QUrl in = QUrl::fromLocalFile("/home");
    bool ret = true;
    {
        ret = model.d->insertFilter(in);
        EXPECT_FALSE(ret);
        EXPECT_TRUE(f1->cc);
        EXPECT_TRUE(f2->cc);
        EXPECT_TRUE(f3->cc);
    }

    {
        ret = false;
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f1->rc = true;
        ret = model.d->insertFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cc);
        EXPECT_FALSE(f2->cc);
        EXPECT_FALSE(f3->cc);
    }

    {
        ret = false;
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f1->rc = false;
        f2->rc = true;
        ret = model.d->insertFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cc);
        EXPECT_TRUE(f2->cc);
        EXPECT_FALSE(f3->cc);
    }

    {
        ret = false;
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f2->rc = false;
        f3->rc = true;
        ret = model.d->insertFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cc);
        EXPECT_TRUE(f2->cc);
        EXPECT_TRUE(f3->cc);
    }
}

TEST_F(TestCanvasModelFilter, renameFilter)
{
    QUrl in = QUrl::fromLocalFile("/home");
    QUrl in2 = QUrl::fromLocalFile("/home2");
    bool ret = true;
    {
        ret = model.d->renameFilter(in, in2);
        EXPECT_FALSE(ret);
        EXPECT_TRUE(f1->cr);
        EXPECT_TRUE(f2->cr);
        EXPECT_TRUE(f3->cr);
    }

    {
        f1->cr = false;
        f2->cr = false;
        f3->cr = false;
        f1->rr = true;
        ret = model.d->renameFilter(in, in2);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cr);
        EXPECT_TRUE(f2->cr);
        EXPECT_TRUE(f3->cr);
    }

    {
        f1->cr = false;
        f2->cr = false;
        f3->cr = false;
        f1->rr = false;
        f2->rr = true;
        ret = model.d->renameFilter(in, in2);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cr);
        EXPECT_TRUE(f2->cr);
        EXPECT_TRUE(f3->cr);
    }

    {
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f2->rr = false;
        f3->rr = true;
        ret = model.d->renameFilter(in, in2);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cr);
        EXPECT_TRUE(f2->cr);
        EXPECT_TRUE(f3->cr);
    }
}

TEST_F(TestCanvasModelFilter, removeFilter)
{
    QUrl in = QUrl::fromLocalFile("/home");
    bool ret = true;

    {
        ret = model.d->removeFilter(in);
        EXPECT_FALSE(ret);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }

    {
        ret = false;
        f1->cd = false;
        f2->cd = false;
        f3->cd = false;
        f1->rd = true;
        ret = model.d->removeFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }

    {
        ret = false;
        f1->cd = false;
        f2->cd = false;
        f3->cd = false;
        f1->rd = false;
        f2->rd = true;
        ret = model.d->removeFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }

    {
        ret = false;
        f1->cd = false;
        f2->cd = false;
        f3->cd = false;
        f2->rd = false;
        f3->rd = true;
        ret = model.d->removeFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }
}

TEST_F(TestCanvasModelFilter, updateFilter)
{
    QUrl in = QUrl::fromLocalFile("/home/test/sss");
    bool ret = true;

    {
        ret = model.d->updateFilter(in, {});
        EXPECT_FALSE(ret);
        EXPECT_TRUE(f1->cu);
        EXPECT_TRUE(f2->cu);
        EXPECT_TRUE(f3->cu);
    }

    {
        ret = false;
        f1->cu = false;
        f2->cu = false;
        f3->cu = false;
        f1->ru = true;
        ret = model.d->updateFilter(in, {});
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cu);
        EXPECT_TRUE(f2->cu);
        EXPECT_TRUE(f3->cu);
    }

    {
        ret = false;
        f1->cu = false;
        f2->cu = false;
        f3->cu = false;
        f1->ru = false;
        f2->ru = true;
        ret = model.d->updateFilter(in, {});
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cu);
        EXPECT_TRUE(f2->cu);
        EXPECT_TRUE(f3->cu);
    }

    {
        ret = false;
        f1->cu = false;
        f2->cu = false;
        f3->cu = false;
        f2->ru = false;
        f3->ru = true;
        ret = model.d->updateFilter(in, {});
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->cu);
        EXPECT_TRUE(f2->cu);
        EXPECT_TRUE(f3->cu);
    }
}

TEST_F(TestCanvasModelFilter, resetFilter)
{
    const QList<QUrl> urls {QUrl::fromLocalFile("/home")};
    bool ret = true;

    {
        auto in = urls;
        ret = model.d->resetFilter(in);
        EXPECT_FALSE(ret);
        EXPECT_TRUE(f1->ct);
        EXPECT_TRUE(f2->ct);
        EXPECT_TRUE(f3->ct);
        EXPECT_FALSE(in.isEmpty());
    }

    {
        auto in = urls;
        ret = false;
        f1->ct = false;
        f2->ct = false;
        f3->ct = false;
        f1->rt = true;
        ret = model.d->resetFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->ct);
        EXPECT_FALSE(f2->ct);
        EXPECT_FALSE(f3->ct);
        EXPECT_TRUE(in.isEmpty());
    }

    {
        auto in = urls;
        ret = false;
        f1->ct = false;
        f2->ct = false;
        f3->ct = false;
        f1->rt = false;
        f2->rt = true;
        ret = model.d->resetFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->ct);
        EXPECT_TRUE(f2->ct);
        EXPECT_FALSE(f3->ct);
        EXPECT_TRUE(in.isEmpty());
    }

    {
        auto in = urls;
        ret = false;
        f1->ct = false;
        f2->ct = false;
        f3->ct = false;
        f2->rt = false;
        f3->rt = true;
        ret = model.d->resetFilter(in);
        EXPECT_TRUE(ret);
        EXPECT_TRUE(f1->ct);
        EXPECT_TRUE(f2->ct);
        EXPECT_TRUE(f3->ct);
        EXPECT_TRUE(in.isEmpty());
    }
}
