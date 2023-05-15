// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/collectionmodel_p.h"
#include "interface/fileinfomodelshell.h"
#include "models/modeldatahandler.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QList>
#include <QUrl>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE

TEST(CollectionModel, clearMapping)
{
    CollectionModel model;
    model.d->fileList.append(QUrl());
    model.d->fileMap.insert(QUrl(), nullptr);

    model.d->clearMapping();
    EXPECT_TRUE(model.d->fileList.isEmpty());
    EXPECT_TRUE(model.d->fileMap.isEmpty());
}

TEST(CollectionModel, rootIndex)
{
    CollectionModel model;
    EXPECT_EQ(model.rootIndex().row(), INT_MAX);
}

TEST(CollectionModel, files)
{
    CollectionModel model;
    model.d->fileList.append(QUrl());
    model.d->fileList.append(QUrl());

    EXPECT_EQ(model.files(), model.d->fileList);
}

TEST(CollectionModel, index)
{
    CollectionModel model;
    QUrl url = QUrl::fromLocalFile("/home");
    model.d->fileList.append(url);
    model.d->fileMap.insert(url, {});

    EXPECT_FALSE(model.index(QUrl()).isValid());
    EXPECT_FALSE(model.index(QUrl::fromLocalFile("/home2")).isValid());

    QModelIndex ret = model.index(QUrl::fromLocalFile("/home"));
    EXPECT_TRUE(ret.isValid());
    EXPECT_EQ(ret.row(), 0);

    ret = model.index(1, 0);
    EXPECT_FALSE(ret.isValid());

    ret = model.index(0, 0);
    EXPECT_TRUE(ret.isValid());
    EXPECT_EQ(ret.row(), 0);
}

TEST(CollectionModel, reset)
{
    CollectionModel model;

    bool arst = false;
    QObject::connect(&model, &CollectionModel::modelAboutToBeReset, &model, [&arst](){
        arst = true;
    });

    bool rst = false;
    QObject::connect(&model, &CollectionModel::modelReset, &model, [&rst](){
        rst = true;
    });

    bool map = false;
    stub_ext::StubExt stub;
    stub.set_lamda(&CollectionModelPrivate::createMapping, [&map](){
        map = true;
    });

    model.d->sourceAboutToBeReset();
    EXPECT_TRUE(arst);
    EXPECT_FALSE(rst);
    EXPECT_FALSE(map);

    model.d->sourceReset();
    EXPECT_TRUE(rst);
    EXPECT_TRUE(map);
}

TEST(CollectionModel, rowCount)
{
    CollectionModel model;
    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);
    EXPECT_EQ(model.rowCount(QModelIndex()), 0);
    EXPECT_EQ(model.rowCount(QModelIndex(0, 0, nullptr, &model)), 0);
    EXPECT_EQ(model.rowCount(model.rootIndex()), 2);
}

TEST(CollectionModel, columnCount)
{
    CollectionModel model;
    EXPECT_EQ(model.columnCount(QModelIndex()), 0);
    EXPECT_EQ(model.columnCount(QModelIndex(0, 0, nullptr, &model)), 0);
    EXPECT_EQ(model.columnCount(model.rootIndex()), 1);
}

TEST(CollectionModel, mimeData)
{
    CollectionModel model;
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

TEST(CollectionModel, take)
{
    CollectionModel model;

    auto in1 = QUrl::fromLocalFile("/home/test");
    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));

    model.d->fileList.append(in1);
    model.d->fileMap.insert(in1, info1);

    int br = -1;
    QObject::connect(&model, &CollectionModel::rowsAboutToBeRemoved, &model, [&br, &model](const QModelIndex &parent, int first, int last) {
        br = first;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, last);
    });
    int er = -1;
    QObject::connect(&model, &CollectionModel::rowsRemoved, &model, [&er, &model](const QModelIndex &parent, int first, int last) {
        er = first;
        EXPECT_EQ(parent, model.rootIndex());
        EXPECT_EQ(first, last);
    });

    {
        EXPECT_TRUE(model.take({QUrl::fromLocalFile("/home/test2")}));
        EXPECT_EQ(br, -1);
        EXPECT_EQ(er, -1);
        EXPECT_EQ(model.d->fileList.size(), 1);
    }

    {
        br = -1;
        er = -1;
        EXPECT_TRUE(model.take({in1}));
        EXPECT_EQ(br, 0);
        EXPECT_EQ(er, 0);
        EXPECT_TRUE(model.d->fileList.isEmpty());
        EXPECT_TRUE(model.d->fileMap.isEmpty());
    }
}

namespace  {
class TestModelHandler : public ModelDataHandler
{
public:
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override {
        reseted = urls;
        return {urls.last()};
    }

    QList<QUrl> reseted;
};

class TestSourceModel : public QAbstractItemModel
{
public:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override{
        return createIndex(row, column, nullptr);
    }
    QModelIndex parent(const QModelIndex &child) const override {
        return QModelIndex();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const {
        return 2;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const {
        return 1;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
        return QVariant();
    }

};

class TestCollectionModel : public testing::Test
{
public:
    void SetUp() override {
        shell.model = &srcModel;
    }

    void TearDown() override {

    }

    stub_ext::StubExt stub;
    TestSourceModel srcModel;
    FileInfoModelShell shell;
    TestModelHandler handler;
};
}

TEST_F(TestCollectionModel, setModelShell)
{
    CollectionModel model;
    model.d->fileList.append(QUrl());
    model.d->fileMap.insert(QUrl(), nullptr);
    bool arst = false;
    QObject::connect(&model, &CollectionModel::modelAboutToBeReset, &model, [&arst](){
        arst = true;
    });

    bool rst = false;
    QObject::connect(&model, &CollectionModel::modelReset, &model, [&rst](){
        rst = true;
    });

    model.setModelShell(&shell);

    EXPECT_EQ(model.sourceModel(), &srcModel);
    EXPECT_TRUE(model.d->fileList.isEmpty());
    EXPECT_TRUE(model.d->fileMap.isEmpty());
    EXPECT_EQ(model.d->shell, &shell);
    EXPECT_TRUE(arst);
    EXPECT_TRUE(rst);

    {
        QUrl t1 = QUrl::fromLocalFile("/tmp/1");
        QUrl t2 = QUrl::fromLocalFile("/tmp/2");
        QUrl u1;
        QUrl u2;
        stub.set_lamda(&CollectionModelPrivate::sourceDataRenamed, [&u1, &u2]
                       (CollectionModelPrivate *, const QUrl &oldUrl, const QUrl &newUrl){
            u1 = oldUrl;
            u2 = newUrl;
        });

        emit shell.dataReplaced(t1, t2);
        EXPECT_EQ(u1, t1);
        EXPECT_EQ(u2, t2);
    }

    {
        bool call = false;
        stub.set_lamda(&CollectionModelPrivate::sourceDataChanged, [&call](){
            call = true;
        });

        emit srcModel.dataChanged(QModelIndex(), QModelIndex());
        EXPECT_TRUE(call);
    }

    {
        bool call = false;
        stub.set_lamda(&CollectionModelPrivate::sourceRowsInserted, [&call](){
            call = true;
        });

        emit srcModel.rowsInserted(QModelIndex(), 0, 0, {});
        EXPECT_TRUE(call);
    }

    {
        bool call = false;
        stub.set_lamda(&CollectionModelPrivate::sourceRowsAboutToBeRemoved, [&call](){
            call = true;
        });

        emit srcModel.rowsAboutToBeRemoved(QModelIndex(), 0, 0, {});
        EXPECT_TRUE(call);
    }

    {
        bool call = false;
        stub.set_lamda(&CollectionModelPrivate::sourceAboutToBeReset, [&call](){
            call = true;
        });

        emit srcModel.modelAboutToBeReset({});
        EXPECT_TRUE(call);
    }

    {
        bool call = false;
        stub.set_lamda(&CollectionModelPrivate::sourceReset, [&call](){
            call = true;
        });

        emit srcModel.modelReset({});
        EXPECT_TRUE(call);
    }
}

TEST_F(TestCollectionModel, createMapping)
{
    CollectionModel model;
    model.d->shell = &shell;
    model.setHandler(&handler);
    model.QAbstractProxyModel::setSourceModel(&srcModel);

    QUrl t1 = QUrl::fromLocalFile("/tmp/1");
    QUrl t2 = QUrl::fromLocalFile("/tmp/2");
    QList<QUrl> files{t1, t2};
    stub.set_lamda(&FileInfoModelShell::files, [&files](){
        return files;
    });
    stub.set_lamda(&FileInfoModelShell::index, [&t2]
                   (FileInfoModelShell *, const QUrl &url, int column){
        EXPECT_EQ(url, t2);
        return QModelIndex(1, 0, nullptr, nullptr);
    });
    stub.set_lamda(&FileInfoModelShell::fileInfo, [](FileInfoModelShell *, const QModelIndex &index){
        EXPECT_EQ(index.row(), 1);
        return FileInfoPointer();
    });

    model.d->createMapping();

    ASSERT_EQ(model.d->fileList.size(), 1);
    EXPECT_EQ(model.d->fileList.first(), t2);
    EXPECT_TRUE(model.d->fileMap.contains(t2));
}

TEST_F(TestCollectionModel,  rootUrl)
{
    CollectionModel model;
    model.d->shell = &shell;
    stub_ext::StubExt stub;
    stub.set_lamda(&FileInfoModelShell::rootUrl, []() {
        return QUrl::fromLocalFile("/home");
    });

    EXPECT_EQ(model.rootUrl(), QUrl::fromLocalFile("/home"));
}

TEST_F(TestCollectionModel, fileUrl)
{
    CollectionModel model;
    model.d->shell = &shell;

    stub_ext::StubExt stub;
    stub.set_lamda(&FileInfoModelShell::rootUrl, []() {
        return QUrl::fromLocalFile("/home");
    });

    QUrl url = QUrl::fromLocalFile("/home/test");
    model.d->fileList.append(url);

    EXPECT_EQ(model.fileUrl(model.rootIndex()), QUrl::fromLocalFile("/home"));
    EXPECT_TRUE(model.fileUrl(QModelIndex()).isEmpty());
    EXPECT_TRUE(model.fileUrl(QModelIndex(1, 0, nullptr, &model)).isEmpty());
    EXPECT_EQ(model.fileUrl(QModelIndex(0, 0, nullptr, &model)), url);
}

TEST_F(TestCollectionModel, sourceDataRenamed)
{
    CollectionModel model;
    model.setHandler(&handler);
    model.QAbstractProxyModel::setSourceModel(&srcModel);
    model.d->shell = &shell;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    QList<QUrl> fileList;
    fileList.append(in1);
    fileList.append(in2);

    stub.set_lamda(&FileInfoModelShell::index, [this, &fileList](FileInfoModelShell *,
                   const QUrl &url, int column) {
        return QModelIndex(fileList.indexOf(url), 0, nullptr, &srcModel);
    });

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    QMap<QUrl, FileInfoPointer> fileMap;
    fileMap.insert(in1, info1);
    fileMap.insert(in2, info2);
    stub.set_lamda(&FileInfoModelShell::fileInfo, [&fileMap, &fileList](FileInfoModelShell *,
                   const QModelIndex &index) {
        return fileMap.value(fileList.at(index.row()));
    });

    bool up = false;
    QObject::connect(&model, &CollectionModel::dataChanged, &model,
                     [&up, &model](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                         up = true;
                         EXPECT_EQ(topLeft.row(), bottomRight.row());
                     });

    QUrl oldu;
    QUrl newu;
    QObject::connect(&model, &CollectionModel::dataReplaced, &model,
                     [&oldu, &newu](const QUrl &t1, const QUrl &t2) {
                         oldu = t1;
                         newu = t2;
                     });

    model.d->fileList.append(in1);
    model.d->fileMap.insert(in1, info1);

    stub_ext::StubExt stub;
    bool filter = true;
    stub.set_lamda(VADDR(ModelDataHandler,acceptRename), [&filter]() {
        return !filter;
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

TEST_F(TestCollectionModel, sourceRowsAboutToBeRemoved)
{
    CollectionModel model;
    model.setHandler(&handler);
    model.QAbstractProxyModel::setSourceModel(&srcModel);
    model.d->shell = &shell;

    auto in1 = QUrl::fromLocalFile("/home/test");
    auto in2 = QUrl::fromLocalFile("/home/test2");
    auto in3 = QUrl::fromLocalFile("/home/test3");
    QList<QUrl> fileList;
    fileList.append(in1);
    fileList.append(in2);
    fileList.append(in3);
    model.d->fileList.append(in1);
    model.d->fileList.append(in2);
    model.d->fileList.append(in3);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    DFMSyncFileInfoPointer info3(new SyncFileInfo(in3));
    QMap<QUrl, FileInfoPointer> fileMap;
    fileMap.insert(in1, info1);
    fileMap.insert(in2, info2);
    fileMap.insert(in3, info3);
    model.d->fileMap.insert(in1, info1);
    model.d->fileMap.insert(in2, info2);
    model.d->fileMap.insert(in3, info3);

    stub_ext::StubExt stub;
    stub.set_lamda(&FileInfoModelShell::fileUrl, [&fileList](FileInfoModelShell *,
                   const QModelIndex &index) {
        return fileList.at(index.row());
    });

    int br = -1;
    int br2 = -1;
    QObject::connect(&model, &CollectionModel::rowsAboutToBeRemoved, &model, [&br, &br2, &model](const QModelIndex &parent, int first, int last) {
        br = first;
        br2 = last;
        EXPECT_EQ(parent, model.rootIndex());
    });

    int er = -1;
    int er2 = -1;
    QObject::connect(&model, &CollectionModel::rowsRemoved, &model, [&er, &er2, &model](const QModelIndex &parent, int first, int last) {
        er = first;
        er2 = last;
        EXPECT_EQ(parent, model.rootIndex());
    });

    {
        model.d->sourceRowsAboutToBeRemoved(QModelIndex(), 1, 1);
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
