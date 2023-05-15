// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/normalized/normalizedmode_p.h"
#include "models/collectionmodel_p.h"
#include "mode/normalized/type/typeclassifier.h"
#include "collection/collectionholder_p.h"
#include "view/collectionwidget_p.h"
#include "view/collectionframe_p.h"
#include "view/collectionview_p.h"
#include "config/configpresenter.h"
#include "interface/canvasviewshell.h"
#include "private/surface.h"

#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

#include <stubext.h>

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE
using namespace testing;

TEST(NormalizedMode, removeClassifier)
{
    NormalizedMode mode;
    CollectionModel model;
    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, []() {
        return ItemCategories(0);
    });

    TypeClassifier *type = new TypeClassifier;
    mode.d->classifier = type;
    ModelDataHandler *handler = type->dataHandler();
    model.d->handler = handler;
    mode.model = &model;

    mode.removeClassifier();
    EXPECT_EQ(mode.d->classifier, nullptr);
    EXPECT_EQ(model.handler(), nullptr);
    EXPECT_NE(mode.d->broker, nullptr);
}

namespace {
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

class TestNormalizedMode : public FileClassifier, public Test
{
public:
    Classifier mode() const {return kName;}
    ModelDataHandler *dataHandler() const {return nullptr;}
    QStringList classes() const {return ids.keys();}
    QString classify(const QUrl &url) const {return ids.key(url.fileName().left(3));}
    QString className(const QString &key) const {return ids.value(key);}
public:
    void SetUp() override {

        stub.set_lamda(&ConfigPresenter::instance, [this](){
            return cfg;
        });

        stub.set_lamda(&ConfigPresenter::updateNormalStyle, [this](){});

        cfg = new ConfigPresenter;
        ids.insert("1", "one");
        ids.insert("2", "two");

        mp = nmode.d;
        nmode.canvasViewShell = &shell;
        nmode.model = &model;
        mp->classifier = this;
    }

    void TearDown() override {
        delete cfg;
        mp->classifier = nullptr;
    }

    stub_ext::StubExt stub;
    ConfigPresenter *cfg;
    QMap<QString, QString> ids;
    CanvasViewShell shell;
    NormalizedMode nmode;
    NormalizedModePrivate *mp;
    CollectionModel model;
};
}

TEST_F(TestNormalizedMode, findValidPos)
{
    QMap<int, QSize> gridSize;
    gridSize.insert(1, QSize(3,3));
    gridSize.insert(2, QSize(4,4));
    nmode.surfaces.append(SurfacePointer());
    nmode.surfaces.append(SurfacePointer());

    stub.set_lamda(&CanvasViewShell::gridSize, [&gridSize](CanvasViewShell *, int viewIndex) {
        return gridSize.value(viewIndex);
    });

    QPoint nextPos(0,0);
    int curIdx = 0;
    CollectionStyle style;
    auto pos = mp->findValidPos(nextPos, curIdx, style, 2, 2);
    EXPECT_EQ(curIdx, 2);
    EXPECT_EQ(pos, QPoint(0,0));
    EXPECT_EQ(nextPos, QPoint(0,2));

    nextPos = QPoint(1, 1);
    curIdx = 1;
    pos = mp->findValidPos(nextPos, curIdx, style, 2, 2);
    EXPECT_EQ(curIdx, 1);
    EXPECT_EQ(pos, QPoint(1, 1));
    EXPECT_EQ(nextPos, QPoint(1,3));

    nextPos = QPoint(1, 2);
    curIdx = 1;
    pos = mp->findValidPos(nextPos, curIdx, style, 2, 2);
    EXPECT_EQ(curIdx, 2);
    EXPECT_EQ(pos, QPoint(0, 0));
    EXPECT_EQ(nextPos, QPoint(0, 2));

    nextPos = QPoint(0, 3);
    curIdx = 2;
    pos = mp->findValidPos(nextPos, curIdx, style, 2, 2);
    EXPECT_EQ(curIdx, 2);
    EXPECT_EQ(pos, QPoint(2, 0));
    EXPECT_EQ(nextPos, QPoint(2, 2));
}

TEST_F(TestNormalizedMode, createCollection)
{
    nmode.surfaces.append(SurfacePointer(new Surface));
    auto ret = mp->createCollection("1");
    ASSERT_NE(ret, nullptr);
    ASSERT_NE(ret->widget(), nullptr);
    ASSERT_NE(ret->frame(), nullptr);
    ASSERT_NE(ret->widget()->view(), nullptr);
    EXPECT_EQ(ret->d->provider, this);
    EXPECT_EQ(ret->surface(), nmode.surfaces.first());
    EXPECT_EQ(ret->id(), QString("1"));
    EXPECT_EQ(ret->name(), QString("one"));
    EXPECT_EQ(ret->widget()->view()->model(), &model);

    EXPECT_FALSE(ret->renamable());
    EXPECT_FALSE(ret->movable());
    EXPECT_FALSE(ret->fileShiftable());
    EXPECT_FALSE(ret->closable());
    EXPECT_FALSE(ret->stretchable());

    EXPECT_TRUE(ret->adjustable());
}

TEST_F(TestNormalizedMode, switchCollection)
{
    QString key;
    CollectionHolderPointer holder(new CollectionHolder("fake", this));
    stub.set_lamda(&NormalizedModePrivate::createCollection, [&key, &holder]
                   (NormalizedModePrivate *,const QString &id){
        key = id;
        return holder;
    });

    bool layout = false;
    stub.set_lamda(VADDR(NormalizedMode,layout), [&layout](){
        layout = true;
    });

    CollectionBaseDataPtr base1(new CollectionBaseData);
    base1->key = "1";
    base1->name = "one";
    this->collections.insert("1", base1);
    // empty item
    {
        mp->switchCollection();
        EXPECT_TRUE(key.isEmpty());
        EXPECT_FALSE(layout);
        EXPECT_TRUE(mp->holders.isEmpty());
    }

    // insert
    {
        mp->holders.clear();;
        key.clear();
        layout = false;
        QUrl one = QUrl::fromLocalFile("/tmp/1");
        base1->items.append(one);
        mp->switchCollection();
        EXPECT_EQ(key, QString("1"));
        EXPECT_TRUE(layout);
        EXPECT_EQ(mp->holders.size(), 1);
        EXPECT_EQ(mp->holders.value("1"), holder);
    }

    // keep
    {
        key.clear();
        layout = false;
        mp->switchCollection();
        EXPECT_TRUE(key.isEmpty());
        EXPECT_FALSE(layout);
        EXPECT_EQ(mp->holders.size(), 1);
        EXPECT_EQ(mp->holders.value("1"), holder);
    }

    // remove
    {
        key.clear();
        layout = false;
        base1->items.clear();
        mp->holders.insert("2", holder);
        mp->switchCollection();
        EXPECT_TRUE(key.isEmpty());
        EXPECT_TRUE(layout);
        EXPECT_EQ(mp->holders.size(), 1);
        EXPECT_EQ(mp->holders.value("1"), nullptr);
        EXPECT_EQ(mp->holders.value("2"), holder);
    }
}

TEST_F(TestNormalizedMode, restore)
{
    CollectionBaseDataPtr base1(new CollectionBaseData);
    base1->key = "1";
    base1->name = "one";
    this->collections.insert("1", base1);
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    base1->items.append(one);

    QUrl two = QUrl::fromLocalFile("/tmp/2");
    base1->items.append(two);

    CollectionBaseDataPtr base2(new CollectionBaseData);
    base2->key = "1";
    base2->name = "one";
    QUrl three = QUrl::fromLocalFile("/tmp/3");
    base2->items.append(three);
    base2->items.append(two);

    mp->restore({base2});
    ASSERT_EQ(base1->items.size(), 2);
    EXPECT_EQ(base1->items.first(), two);
    EXPECT_EQ(base1->items.last(), one);
}

TEST_F(TestNormalizedMode, initialize)
{
    Classifier type = kName;
    stub.set_lamda(&ConfigPresenter::classification, [&type](){
        return type;
    });

    Classifier iType;
    stub.set_lamda(&NormalizedMode::setClassifier, [&iType](NormalizedMode *, Classifier id){
        iType = id;
        return true;
    });

    bool rebuild = false;
    stub.set_lamda(&NormalizedMode::rebuild, [&rebuild](){
        rebuild = true;
    });

    EXPECT_TRUE(nmode.initialize(&model));
    EXPECT_EQ(iType, type);
    EXPECT_FALSE(rebuild);

    rebuild = false;
    type = kSize;
    model.d->fileList.append(QUrl());
    EXPECT_TRUE(nmode.initialize(&model));
    EXPECT_EQ(iType, type);
    EXPECT_TRUE(rebuild);
}
