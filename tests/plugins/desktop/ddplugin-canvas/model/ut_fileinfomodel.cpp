// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/fileinfomodel_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QStandardPaths>
#include <QMimeData>


DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(FileInfoModel, construct)
{
    FileInfoModel model;
    ASSERT_NE(model.d->fileProvider, nullptr);

    stub_ext::StubExt stub;

    // check signal
    bool re = false;
    stub.set_lamda(&FileInfoModelPrivate::resetData, [&re](){
        re = true;
    });
    emit model.d->fileProvider->refreshEnd({});
    EXPECT_TRUE(re);

    bool ins = false;
    stub.set_lamda(&FileInfoModelPrivate::insertData, [&ins](){
        ins = true;
    });
    emit model.d->fileProvider->fileInserted(QUrl());
    EXPECT_TRUE(ins);

    bool rm = false;
    stub.set_lamda(&FileInfoModelPrivate::removeData, [&rm](){
        rm = true;
    });
    emit model.d->fileProvider->fileRemoved(QUrl());
    EXPECT_TRUE(rm);

    bool up = false;
    stub.set_lamda(&FileInfoModelPrivate::updateData, [&up](){
        up = true;
    });
    emit model.d->fileProvider->fileUpdated(QUrl());
    EXPECT_TRUE(up);

    bool rep = false;
    stub.set_lamda(&FileInfoModelPrivate::replaceData, [&rep](){
        rep = true;
    });
    emit model.d->fileProvider->fileRenamed(QUrl(), QUrl());
    EXPECT_TRUE(rep);
}

TEST(FileInfoModel, setRootUrl)
{
    FileInfoModel model;
    stub_ext::StubExt stub;
    bool refresh = false;
    stub.set_lamda(&FileProvider::refresh, [&refresh](){
        refresh = true;
    });

    QUrl root;
    stub.set_lamda(&FileProvider::setRoot, [&root](FileProvider *,const QUrl &url){
        root = url;
        return true;
    });

    auto idx = model.setRootUrl(QUrl());
    EXPECT_EQ(idx, model.createIndex(INT_MAX, 0, &model));
    EXPECT_EQ(model.modelState(), 0x2);
    EXPECT_EQ(root.toLocalFile(), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    EXPECT_EQ(model.d->filters, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden);

    auto in = QUrl::fromLocalFile("/home");
    idx = model.setRootUrl(in);
    EXPECT_EQ(root, in);
}

TEST(FileInfoModel, rootUrl)
{
    FileInfoModel model;
    EXPECT_EQ(model.rootUrl(), QUrl());

    auto in = QUrl::fromLocalFile("/home");
    model.d->fileProvider->rootUrl = in;
    EXPECT_EQ(model.rootUrl(), in);
}

TEST(FileInfoModel, index)
{
    auto in = QUrl::fromLocalFile("/home");
    DFMSyncFileInfoPointer info(new SyncFileInfo(in));
    FileInfoModel model;
    model.d->fileList.append(in);
    model.d->fileMap.insert(in, info);

    auto idx = model.index(-1, 0);
    EXPECT_FALSE(idx.isValid());

    idx = model.index(0, -1);
    EXPECT_FALSE(idx.isValid());

    idx = model.index(-1, -1);
    EXPECT_FALSE(idx.isValid());

    idx = model.index(0, 0);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);

    idx = model.index(1, 0);
    EXPECT_FALSE(idx.isValid());
}

TEST(FileInfoModel, index2)
{
    auto in = QUrl::fromLocalFile("/home/test");
    DFMSyncFileInfoPointer info(new SyncFileInfo(in));
    FileInfoModel model;
    model.d->fileProvider->rootUrl = QUrl::fromLocalFile("/home");
    model.d->fileList.append(in);
    model.d->fileMap.insert(in, info);

    auto idx = model.index(QUrl());
    EXPECT_FALSE(idx.isValid());

    idx = model.index(in);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);

    idx = model.index(QUrl::fromLocalFile("/home/test2"));
    EXPECT_FALSE(idx.isValid());

    idx = model.index(QUrl::fromLocalFile("/home"));
    EXPECT_EQ(idx, model.rootIndex());
}

TEST(FileInfoModel, fileInfo)
{
    auto in = QUrl::fromLocalFile("/home/test");
    DFMSyncFileInfoPointer info(new SyncFileInfo(in));
    FileInfoModel model;
    model.d->fileProvider->rootUrl = QUrl::fromLocalFile("/home");
    model.d->fileList.append(in);
    model.d->fileMap.insert(in, info);

    auto local = model.fileInfo(model.rootIndex());
    EXPECT_EQ(local->urlOf(SyncFileInfo::FileUrlInfoType::kUrl), model.d->fileProvider->rootUrl);

    local = model.fileInfo(QModelIndex(-1, 0, nullptr, nullptr));
    EXPECT_EQ(local, nullptr);

    local = model.fileInfo(QModelIndex(1, 0, nullptr, nullptr));
    EXPECT_EQ(local, nullptr);

    local = model.fileInfo(QModelIndex(0, 0, nullptr, nullptr));
    EXPECT_EQ(local, info);
}

TEST(FileInfoModel, fileUrl)
{
    auto in = QUrl::fromLocalFile("/home/test");
    FileInfoModel model;
    model.d->fileProvider->rootUrl = QUrl::fromLocalFile("/home");
    model.d->fileList.append(in);

    auto local = model.fileUrl(model.rootIndex());
    EXPECT_EQ(local, model.d->fileProvider->rootUrl);

    local = model.fileUrl(QModelIndex(-1, 0, nullptr, nullptr));
    EXPECT_TRUE(local.isEmpty());

    local = model.fileUrl(QModelIndex(1, 0, nullptr, nullptr));
    EXPECT_TRUE(local.isEmpty());

    local = model.fileUrl(QModelIndex(0, 0, nullptr, nullptr));
    EXPECT_EQ(local, in);
}

TEST(FileInfoModel, refresh)
{
    FileInfoModel model;
    stub_ext::StubExt stub;
    bool refresh = false;
    stub.set_lamda(&FileProvider::refresh, [&refresh](){
        refresh = true;
    });

    model.refresh(QModelIndex());
    EXPECT_FALSE(refresh);
    EXPECT_EQ(model.modelState(), 0);

    refresh = false;
    model.refresh(model.rootIndex());
    EXPECT_TRUE(refresh);
    EXPECT_EQ(model.modelState(), 0x2);
}

TEST(FileInfoModel, update)
{
    FileInfoModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));

    model.d->fileMap.insert(in1, info1);
    model.d->fileMap.insert(in2, info2);

    stub_ext::StubExt stub;
    int refresh = 0;
    stub.set_lamda(VADDR(SyncFileInfo,refresh), [&refresh](){
        refresh++;
    });

    QModelIndex t1;
    QModelIndex t2;
    QObject::connect(&model, &FileInfoModel::dataChanged, &model,
                     [&t1, &t2](const QModelIndex &topLeft, const QModelIndex &bottomRight){
        t1 = topLeft;
        t2 = bottomRight;
    });

    model.update();

    EXPECT_EQ(refresh, 2);
    EXPECT_EQ(t1.row(), 0);
    EXPECT_EQ(t2.row(), 1);
}

TEST(FileInfoModel, updateFile)
{
    FileInfoModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(in1);
    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    model.d->fileMap.insert(in1, info1);

    stub_ext::StubExt stub;
    int refresh = 0;
    stub.set_lamda(VADDR(SyncFileInfo,refresh), [&refresh](){
        refresh++;
    });

    QModelIndex t1;
    QModelIndex t2;
    QObject::connect(&model, &FileInfoModel::dataChanged, &model,
                     [&t1, &t2](const QModelIndex &topLeft, const QModelIndex &bottomRight){
        t1 = topLeft;
        t2 = bottomRight;
    });

    model.updateFile(QUrl::fromLocalFile("/home/test2"));
    EXPECT_EQ(refresh, 0);
    EXPECT_FALSE(t1.isValid());
    EXPECT_FALSE(t2.isValid());

    refresh = false;
    model.updateFile(in1);
    EXPECT_TRUE(t1.isValid());
    EXPECT_TRUE(t2.isValid());
    EXPECT_EQ(t1.row(), 0);
    EXPECT_EQ(t2.row(), 0);
}

TEST(FileInfoModel, rowCount)
{
    FileInfoModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(in1);

    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_EQ(model.rowCount(model.index(0)), 0);
    EXPECT_EQ(model.rowCount(model.rootIndex()), 1);
}

TEST(FileInfoModel, columnCount)
{
    FileInfoModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(in1);

    EXPECT_EQ(model.columnCount(), 0);
    EXPECT_EQ(model.columnCount(model.index(0)), 0);
    EXPECT_EQ(model.columnCount(model.rootIndex()), 1);
}

TEST(FileInfoModel, mimeData)
{
    FileInfoModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(in1);
    model.d->fileMap.insert(in1, {});

    auto mm = model.mimeData({model.index(0)});
    ASSERT_NE(mm, nullptr);

    auto urls = mm->urls();
    ASSERT_EQ(urls.size(), 1);
    EXPECT_EQ(urls.first(), in1);
}

TEST(FileInfoModel, mimeTypes)
{
    FileInfoModel model;
    ASSERT_EQ(model.mimeTypes().size(), 1);
    EXPECT_EQ(model.mimeTypes().first(), QLatin1String("text/uri-list"));
}

TEST(FileInfoModel, flags)
{
    FileInfoModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(in1);
    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    model.d->fileMap.insert(in1, info1);

    EXPECT_EQ(model.flags(QModelIndex()), model.QAbstractItemModel::flags(QModelIndex()));

    stub_ext::StubExt stub;
    bool canRenmae = false;;
    bool canDrop = false;
    stub.set_lamda(VADDR(SyncFileInfo,canAttributes), [&canRenmae, &canDrop](SyncFileInfo *, const SyncFileInfo::FileCanType type){
        if (type == SyncFileInfo::FileCanType::kCanRename)
            return canRenmae;
        else if (type == SyncFileInfo::FileCanType::kCanDrop)
            return canDrop;
        return true;
    });

    bool isWrite = false;
    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [&isWrite](SyncFileInfo *, const SyncFileInfo::FileIsType type){
        if (type == SyncFileInfo::FileIsType::kIsWritable)
            return isWrite;

        return true;
    });

    auto idx = model.index(0);
    {
        auto f = model.flags(idx);
        EXPECT_FALSE(f.testFlag(Qt::ItemIsEditable));
        EXPECT_FALSE(f.testFlag(Qt::ItemIsDropEnabled));
        EXPECT_FALSE(f.testFlag(Qt::ItemNeverHasChildren));
    }

    {
        canRenmae = true;
        auto f = model.flags(idx);
        EXPECT_TRUE(f.testFlag(Qt::ItemIsEditable));
        EXPECT_FALSE(f.testFlag(Qt::ItemIsDropEnabled));
        EXPECT_FALSE(f.testFlag(Qt::ItemNeverHasChildren));

    }

    {
        canRenmae = true;
        isWrite = true;
        auto f = model.flags(idx);
        EXPECT_TRUE(f.testFlag(Qt::ItemIsEditable));
        EXPECT_FALSE(f.testFlag(Qt::ItemIsDropEnabled));
        EXPECT_TRUE(f.testFlag(Qt::ItemNeverHasChildren));
    }


    {
        canRenmae = true;
        isWrite = true;
        canDrop = true;
        auto f = model.flags(idx);
        EXPECT_TRUE(f.testFlag(Qt::ItemIsEditable));
        EXPECT_TRUE(f.testFlag(Qt::ItemIsDropEnabled));
        EXPECT_FALSE(f.testFlag(Qt::ItemNeverHasChildren));
    }
}

TEST(FileInfoModelPrivate, resetData)
{
    FileInfoModel model;
    auto in1 = QUrl::fromLocalFile("/home/test");

    bool be = false;
    QObject::connect(&model, &FileInfoModel::modelAboutToBeReset, &model,
                     [&be](){
        be = true;
    });

    bool end = false;
    QObject::connect(&model, &FileInfoModel::modelReset, &model,
                     [&end](){
        end = true;
    });

    model.d->modelState = FileInfoModelPrivate::RefreshState;
    model.d->resetData({in1});
    EXPECT_TRUE(be);
    EXPECT_TRUE(end);
    EXPECT_EQ(model.d->fileList.size(), 1);
    EXPECT_TRUE(model.d->fileMap.contains(in1));
    EXPECT_EQ(model.modelState(), 1);
}

TEST(FileInfoModelPrivate, insertData)
{
    FileInfoModel model;
    auto in1 = QUrl::fromLocalFile("/home/test");

    bool be = false;
    QObject::connect(&model, &FileInfoModel::rowsAboutToBeInserted, &model,
                     [&be, &model](const QModelIndex &parent, int first, int last){
        be = true;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, 0);
        EXPECT_EQ(last, 0);
    });

    bool end = false;
    QObject::connect(&model, &FileInfoModel::rowsInserted, &model,
                     [&end](){
        end = true;
    });

    model.d->insertData(in1);
    EXPECT_TRUE(be);
    EXPECT_TRUE(end);
    EXPECT_EQ(model.d->fileList.size(), 1);
    ASSERT_TRUE(model.d->fileMap.contains(in1));

    be = false;
    end = false;
    model.d->insertData(in1);
    EXPECT_FALSE(be);
    EXPECT_FALSE(end);
    EXPECT_EQ(model.d->fileList.size(), 1);
    ASSERT_TRUE(model.d->fileMap.contains(in1));
}

TEST(FileInfoModelPrivate, removeData)
{
    FileInfoModel model;
    auto in1 = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(in1);
    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    model.d->fileMap.insert(in1, info1);

    bool be = false;
    QObject::connect(&model, &FileInfoModel::rowsAboutToBeRemoved, &model,
                     [&be, &model](const QModelIndex &parent, int first, int last){
        be = true;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, 0);
        EXPECT_EQ(last, 0);
    });

    bool end = false;
    QObject::connect(&model, &FileInfoModel::rowsRemoved, &model,
                     [&end](){
        end = true;
    });

    model.d->removeData(QUrl::fromLocalFile("/home/test2"));
    EXPECT_FALSE(be);
    EXPECT_FALSE(end);
    EXPECT_EQ(model.d->fileList.size(), 1);
    ASSERT_TRUE(model.d->fileMap.contains(in1));

    be = false;
    end = false;
    model.d->removeData(in1);
    EXPECT_TRUE(be);
    EXPECT_TRUE(end);
    EXPECT_TRUE(model.d->fileList.isEmpty());
}

TEST(FileInfoModelPrivate, replaceData)
{
    FileInfoModel model;
    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));

    bool up = false;
    QObject::connect(&model, &FileInfoModel::dataChanged, &model,
                     [&up, &model](const QModelIndex &topLeft, const QModelIndex &bottomRight){
        up = true;
        EXPECT_EQ(topLeft.row(), bottomRight.row());
    });

    QUrl oldu;
    QUrl newu;
    QObject::connect(&model, &FileInfoModel::dataReplaced, &model,
                     [&oldu, &newu](const QUrl &t1, const QUrl &t2){
        oldu = t1;
        newu = t2;
    });

    model.d->fileList.append(in1);
    model.d->fileMap.insert(in1, info1);

    stub_ext::StubExt stub;
    QUrl rd;
    stub.set_lamda(&FileInfoModelPrivate::removeData, [&rd](FileInfoModelPrivate *, const QUrl &url){
        rd = url;
    });

    QUrl ins;
    stub.set_lamda(&FileInfoModelPrivate::insertData, [&ins](FileInfoModelPrivate *, const QUrl &url){
        ins = url;
    });


    // move a to null
    {
        model.d->replaceData(in1, QUrl());
        EXPECT_FALSE(up);
        EXPECT_TRUE(ins.isEmpty());
        EXPECT_EQ(rd, in1);
        EXPECT_TRUE(oldu.isEmpty());
        EXPECT_TRUE(newu.isEmpty());
    }

    // move a to b, a is not existed
    {
        model.d->fileList.clear();
        model.d->fileMap.clear();

        up = false;
        ins = QUrl();
        rd = QUrl();
        oldu = QUrl();
        newu = QUrl();
        model.d->replaceData(in1, in2);
        EXPECT_FALSE(up);
        EXPECT_TRUE(rd.isEmpty());
        EXPECT_EQ(ins, in2);
        EXPECT_TRUE(oldu.isEmpty());
        EXPECT_TRUE(newu.isEmpty());
    }

    // move a to b, b is existed
    {
        model.d->fileList.append(in1);
        model.d->fileMap.insert(in1, info1);
        model.d->fileList.append(in2);
        model.d->fileMap.insert(in2, info2);

        up = false;
        ins = QUrl();
        rd = QUrl();
        oldu = QUrl();
        newu = QUrl();
        model.d->replaceData(in1, in2);

        EXPECT_EQ(rd, in1);
        EXPECT_TRUE(ins.isEmpty());
        EXPECT_EQ(oldu, in1);
        EXPECT_EQ(newu, in2);
        EXPECT_TRUE(up);
    }

    // move a to b, b is not existed
    {
        model.d->fileList.clear();
        model.d->fileMap.clear();
        model.d->fileList.append(in1);
        model.d->fileMap.insert(in1, info1);

        up = false;
        ins = QUrl();
        rd = QUrl();
        oldu = QUrl();
        newu = QUrl();
        model.d->replaceData(in1, in2);

        EXPECT_TRUE(rd.isEmpty());
        EXPECT_TRUE(ins.isEmpty());

        EXPECT_EQ(oldu, in1);
        EXPECT_EQ(newu, in2);
        EXPECT_TRUE(up);
        ASSERT_EQ(model.d->fileList.size(), 1);
        EXPECT_EQ(model.d->fileList.first(), in2);
        ASSERT_EQ(model.d->fileMap.size(), 1);
        EXPECT_TRUE(model.d->fileMap.contains(in2));
    }
}
