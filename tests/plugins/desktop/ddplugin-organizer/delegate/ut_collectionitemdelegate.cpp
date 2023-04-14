// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delegate/collectionitemdelegate.h"
#include "delegate/collectionitemdelegate_p.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"
#include "models/collectionmodel.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPainter>
#include <QRectF>
#include <QUrl>

DPF_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_organizer;

class CollectionItemDelegateTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CollectionItemDelegateTest, updateItemSizeHint)
{

    CollectionView view(QString("testuuid"), nullptr);
    CollectionItemDelegate obj(&view);

    int height = 20;
    stub.set_lamda(&QFontMetrics::height, [&height]() {
        return height;
    });

    QSize icon = QSize(50, 50);
    stub.set_lamda(&CollectionView::iconSize, [&icon]() {
        return icon;
    });

    obj.updateItemSizeHint();
    EXPECT_EQ(obj.d->textLineHeight, height);
    EXPECT_EQ(obj.d->itemSizeHint.width(), icon.width() * 17 / 10);
    EXPECT_EQ(obj.d->itemSizeHint.height(), icon.height() + 10 + 2 * height);

    height = 30;
    icon = QSize(60, 60);
    obj.updateItemSizeHint();
    EXPECT_EQ(obj.d->textLineHeight, height);
    EXPECT_EQ(obj.d->itemSizeHint.width(), icon.width() * 17 / 10);
    EXPECT_EQ(obj.d->itemSizeHint.height(), icon.height() + 10 + 2 * height);
}

TEST_F(CollectionItemDelegateTest, paintEmblems)
{
    QString inspace;
    QString intopic;
    QPainter *inpainter = nullptr;
    QRectF inRect;
    QUrl inurl;
    stub.set_lamda((QVariant(EventChannelManager::*)(const QString &, const QString &, QPainter *, const QRectF &, const QUrl &))
                           & EventChannelManager::push,
                   [&](EventChannelManager *, const QString &space, const QString &topic, QPainter *p, const QRectF &r, const QUrl &url) {
                       inspace = space;
                       intopic = topic;
                       inpainter = p;
                       inRect = r;
                       inurl = url;
                       return QVariant::fromValue(true);
                   });

    QPainter p;
    QRectF r(10, 10, 10, 10);
    QUrl url = QUrl::fromLocalFile("/usr");
    FileInfoPointer info = DFMBASE_NAMESPACE::InfoFactory::create<DFMBASE_NAMESPACE::FileInfo>(url);
    CollectionItemDelegate::paintEmblems(&p, r, info);

    EXPECT_EQ(inspace, QString("dfmplugin_emblem"));
    EXPECT_EQ(intopic, QString("slot_FileEmblems_Paint"));
    EXPECT_EQ(inpainter, &p);
    EXPECT_EQ(inRect, r);
    EXPECT_EQ(inurl, url);
}

TEST_F(CollectionItemDelegateTest, paintLabel)
{
    CollectionModel model;
    CollectionView view(QString("testuuid"), nullptr);
    view.setModel(&model);

    CollectionItemDelegate delgate(&view);

    bool isHighlight = true;
    stub.set_lamda(&CollectionItemDelegatePrivate::isHighlight, [&isHighlight]() {
        return isHighlight;
    });

    stub.set_lamda(&CollectionModel::fileUrl, []() {
        return QUrl::fromLocalFile("/usr");
    });

    bool drawHighlightText = false;
    stub.set_lamda(&CollectionItemDelegate::drawHighlightText, [&drawHighlightText]() {
        drawHighlightText = true;
    });

    bool drawNormlText = false;
    stub.set_lamda(&CollectionItemDelegate::drawNormlText, [&drawNormlText]() {
        drawNormlText = true;
    });

    QPainter painter;
    QRect rect(10, 10, 100, 100);
    delgate.paintLabel(&painter, QStyleOptionViewItem(), QModelIndex(), rect);

    EXPECT_TRUE(drawHighlightText);
    EXPECT_FALSE(drawNormlText);

    isHighlight = false;
    drawNormlText = false;
    drawHighlightText = false;

    delgate.paintLabel(&painter, QStyleOptionViewItem(), QModelIndex(), rect);

    EXPECT_FALSE(drawHighlightText);
    EXPECT_TRUE(drawNormlText);
}

//TEST_F(CollectionItemDelegateTest, extendPaintText)
//{
//    QString inspace;
//    QString intopic;
//    QPainter *inpainter = nullptr;
//    QRectF inRect;
//    QUrl inurl;
//    int inrole = -1;
//    auto func = (bool (EventSequenceManager::*)(const QString &, const QString &,
//                                                          const int, const QUrl &, QPainter *, QRectF *))
//                       &EventSequenceManager::run;
//    stub.set_lamda(func,
//                   [&](EventSequenceManager *, const QString &space, const QString &topic
//                   , const int role, const QUrl &url, QPainter *p, QRectF *r){
//        inspace = space;
//        intopic = topic;
//        inpainter = p;
//        inRect = *r;
//        inurl = url;
//        inrole = role;
//        return true;
//    });
//    QPainter p;
//    QRectF r(10, 10, 10, 10);
//    QUrl url = QUrl::fromLocalFile("/usr");
//    CollectionItemDelegate::extendPaintText(&p, url, &r);

//    EXPECT_EQ(inspace, QString("ddplugin_canvas"));
//    EXPECT_EQ(intopic, QString("hook_CanvasItemDelegate_PaintText"));
//    EXPECT_EQ(inpainter, &p);
//    EXPECT_EQ(inRect, r);
//    EXPECT_EQ(inurl, url);
//    EXPECT_EQ(inrole, dfmbase::Global::ItemRoles::kItemFileDisplayNameRole);
//}

TEST(CollectionItemDelegate, iconLevelRange)
{
    QList<int> iconSizes = { 32, 48, 64, 96, 128 };
    ASSERT_EQ(CollectionItemDelegatePrivate::kIconSizes, iconSizes);

    EXPECT_EQ(CollectionItemDelegate::minimumIconLevel(), 0);
    EXPECT_EQ(CollectionItemDelegate::maximumIconLevel(), 4);
}

TEST(CollectionItemDelegatePrivate, needExpend)
{
    CollectionView view(QString("testuuid"), nullptr);
    CollectionItemDelegate obj(&view);

    stub_ext::StubExt stub;
    stub.set_lamda(&CollectionItemDelegate::textPaintRect, []() {
        return QRect(100, 100, 100, 200);
    });

    QRect out;
    QRect in(10, 10, 100, 100);
    const QRect need(10, 10, 100, 200);
    EXPECT_TRUE(obj.d->needExpend({}, {}, in, &out));
    EXPECT_EQ(out, need);

    in = QRect(10, 10, 100, 300);
    EXPECT_FALSE(obj.d->needExpend({}, {}, in, &out));
    EXPECT_EQ(out, need);
}
