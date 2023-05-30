// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delegate/collectionitemdelegate_p.h"
#include "view/collectionview_p.h"
#include "models/collectionmodel_p.h"
#include "delegate/itemeditor.h"
#include "interface/fileinfomodelshell.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/deviceutils.h"

#include <dfm-framework/dpf.h>

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPainter>
#include <QRectF>
#include <QUrl>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_organizer;

TEST(CollectionItemDelegate, paint)
{
    CollectionModel model;
    CollectionView view("1", nullptr);
    view.setModel(&model);
    CollectionItemDelegate obj(&view);

    stub_ext::StubExt stub;
    bool opt = false;;
    stub.set_lamda(VADDR(CollectionItemDelegate,initStyleOption), [&opt](){
        opt = true;
    });

    stub.set_lamda(&CollectionItemDelegate::isTransparent, [](){
        return false;
    });


    bool icon = false;
    stub.set_lamda(&CollectionItemDelegate::paintIcon, [&icon](){
        icon = true;
        return QRect();
    });
    bool emblem = false;
    stub.set_lamda(&CollectionItemDelegate::paintEmblems, [&emblem](){
        emblem = true;
        return QRectF();
    });
    bool label = false;
    stub.set_lamda(&CollectionItemDelegate::paintLabel, [&label](){
        label = true;
    });

    stub.set_lamda(&CollectionView::isPersistentEditorOpen, [](){
        return false;
    });

    QPixmap pix(1920,1080);
    QPainter pa(&pix);
    QStyleOptionViewItem indexOption;
    indexOption.rect = QRect(50,50,200,200);
    obj.paint(&pa, indexOption, QModelIndex());
    EXPECT_TRUE(opt);
    EXPECT_TRUE(icon);
    EXPECT_TRUE(emblem);
    EXPECT_TRUE(label);
}

TEST(CollectionItemDelegate, createTextlayout)
{
    CollectionView view("1", nullptr);
    CollectionItemDelegate obj(&view);

    stub_ext::StubExt stub;
    stub.set_lamda(&QModelIndex::data, [](QModelIndex *, int arole) -> QVariant{
        if (arole == Global::ItemRoles::kItemFileDisplayNameRole)
            return QString("test.log");
        else if (arole == Global::ItemRoles::kItemFileBaseNameOfRenameRole)
            return QString("test");
        else
            return QVariant();
    });

    bool suffix = false;
    stub.set_lamda(&Application::genericAttribute, [&suffix]() {
        return QVariant(suffix);
    });

    QPainter pa;

    obj.d->textLineHeight = 11;
    auto lay = obj.d->createTextlayout(QModelIndex(0, 0, nullptr, nullptr), &pa);
    EXPECT_EQ(lay->text(), QString("test"));
    EXPECT_EQ(lay->attribute<int>(ElideTextLayout::kLineHeight), 11);
    EXPECT_EQ(lay->attribute<int>(ElideTextLayout::kAlignment), Qt::AlignCenter);
    EXPECT_EQ(lay->attribute<uint>(ElideTextLayout::kWrapMode), (uint)QTextOption::WrapAnywhere);
    EXPECT_EQ(lay->attribute<QFont>(ElideTextLayout::kFont), pa.font());
    EXPECT_EQ(lay->attribute<Qt::LayoutDirection>(ElideTextLayout::kTextDirection), pa.layoutDirection());
    delete lay;

    suffix = true;
    lay = obj.d->createTextlayout(QModelIndex(0, 0, nullptr, nullptr), &pa);
    EXPECT_EQ(lay->text(), QString("test.log"));
    delete lay;
}

TEST(CollectionItemDelegate, updateItemSizeHint)
{
    stub_ext::StubExt stub;
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

TEST(CollectionItemDelegate, paintEmblems)
{
    stub_ext::StubExt stub;
    QString inspace;
    QString intopic;
    QPainter *inpainter = nullptr;
    QRectF inRect;
    FileInfoPointer inFile;
    stub.set_lamda((QVariant(EventChannelManager::*)(const QString &, const QString &, QPainter *, const QRectF &, const FileInfoPointer &))
                           & EventChannelManager::push,
                   [&](EventChannelManager *, const QString &space, const QString &topic, QPainter *p, const QRectF &r, const FileInfoPointer &file) {
                       inspace = space;
                       intopic = topic;
                       inpainter = p;
                       inRect = r;
                       inFile = file;
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
    EXPECT_EQ(inFile, info);
}

TEST(CollectionItemDelegate, paintLabel)
{
    stub_ext::StubExt stub;
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
//    EXPECT_EQ(intopic, QString("hook_CollectionItemDelegate_PaintText"));
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

TEST(CollectionItemDelegatePrivate, setEditorData)
{
    CollectionView view(QString("testuuid"), nullptr);
    CollectionItemDelegate obj(&view);

    stub_ext::StubExt stub;
    stub.set_lamda(&QModelIndex::data, [](QModelIndex *, int arole) -> QVariant{
        if (arole == Global::ItemRoles::kItemFileNameOfRenameRole)
            return QString("test.log");
        else if (arole == Global::ItemRoles::kItemFileBaseNameOfRenameRole)
            return QString("test");
        else if (arole == Global::ItemRoles::kItemFileSuffixOfRenameRole)
            return QString("log");
        else
            return QVariant();
    });

    bool suffix = false;
    stub.set_lamda(&Application::genericAttribute, [&suffix]() {
        return QVariant(suffix);
    });

    QString str;
    stub.set_lamda(&ItemEditor::select, [&str](ItemEditor *, const QString &part) {
        str = part;
    });

    {
        ItemEditor editor;
        obj.setEditorData(&editor, QModelIndex());
        EXPECT_EQ(editor.property("_d_whether_show_suffix").toString(), QString("log"));
        EXPECT_EQ(editor.maximumLength(), NAME_MAX - 4);
        EXPECT_EQ(editor.text(), QString("test"));
        EXPECT_EQ(str, editor.text());
    }

    {
        suffix = true;
        str.clear();
        ItemEditor editor;
        obj.setEditorData(&editor, QModelIndex());
        EXPECT_EQ(editor.property("_d_whether_show_suffix").toString(), QString());
        EXPECT_EQ(editor.maximumLength(), NAME_MAX);
        EXPECT_EQ(editor.text(), QString("test.log"));
        EXPECT_EQ(str, QString("test"));
    }
}

TEST(CollectionItemDelegatePrivate, setEditorData_nosuffix)
{
    CollectionView view(QString("testuuid"), nullptr);
    CollectionItemDelegate obj(&view);

    stub_ext::StubExt stub;
    stub.set_lamda(&QModelIndex::data, [](QModelIndex *, int arole) -> QVariant{
        if (arole == Global::ItemRoles::kItemFileNameOfRenameRole)
            return QString("test");
        else if (arole == Global::ItemRoles::kItemFileBaseNameOfRenameRole)
            return QString("test");
        else if (arole == Global::ItemRoles::kItemFileSuffixOfRenameRole)
            return QString("");
        else
            return QVariant();
    });

    bool suffix = false;
    stub.set_lamda(&Application::genericAttribute, [&suffix]() {
        return QVariant(suffix);
    });

    QString str;
    stub.set_lamda(&ItemEditor::select, [&str](ItemEditor *, const QString &part) {
        str = part;
    });

    {
        ItemEditor editor;
        obj.setEditorData(&editor, QModelIndex());
        EXPECT_EQ(editor.property("_d_whether_show_suffix").toString(), QString(""));
        EXPECT_EQ(editor.maximumLength(), NAME_MAX);
        EXPECT_EQ(editor.text(), QString("test"));
        EXPECT_EQ(str, editor.text());
    }

    {
        suffix = true;
        str.clear();
        ItemEditor editor;
        obj.setEditorData(&editor, QModelIndex());
        EXPECT_EQ(editor.property("_d_whether_show_suffix").toString(), QString());
        EXPECT_EQ(editor.maximumLength(), NAME_MAX);
        EXPECT_EQ(editor.text(), QString("test"));
        EXPECT_EQ(str, editor.text());
    }
}

TEST(CollectionItemDelegatePrivate, setModelData)
{
    CollectionView view(QString("testuuid"), nullptr);
    CollectionModel model;
    CollectionItemDelegate obj(&view);

    ItemEditor editor;
    stub_ext::StubExt stub;
    stub.set_lamda(&QModelIndex::data, [](QModelIndex *, int arole) -> QVariant{
        if (arole == Global::ItemRoles::kItemFileNameOfRenameRole)
            return QString("test.log");
        else
            return QVariant();
    });

    bool ct = false;
    stub.set_lamda(&CollectionModel::fileInfo, [&ct]() {
        ct = true;
        return FileInfoPointer();
    });

    {
        editor.setText("test");
        editor.setProperty("_d_whether_show_suffix", "log");
        obj.setModelData(&editor, &model, QModelIndex());
        EXPECT_FALSE(ct);
    }

    {
        ct = false;
        editor.setText("test2");
        editor.setProperty("_d_whether_show_suffix", "log");
        obj.setModelData(&editor, &model, QModelIndex());
        EXPECT_TRUE(ct);
    }
}

TEST(CollectionItemDelegate, boundingRect)
{
    QList<QRectF> rects;
    rects.append(QRectF(200, 200, 20, 20));
    rects.append(QRectF(210, 200, 20, 20));
    rects.append(QRectF(200, 210, 20, 20));
    rects.append(QRectF(190, 190, 20, 20));

    auto ret = CollectionItemDelegate::boundingRect(rects);
    EXPECT_EQ(ret, QRectF(190, 190, 40, 40));
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

class TestCollectionItemDelegate : public testing::Test
{
public:
    void SetUp() override
    {
        shell.model = &srcModel;

        model = new CollectionModel;
        model->d->shell = &shell;
        model->QAbstractProxyModel::setSourceModel(&srcModel);

        view = new CollectionView("1", nullptr);
        view->setModel(model);

        dlgt = new CollectionItemDelegate(view);

        auto in1 = QUrl::fromLocalFile("/tmp/1.txt");
        auto in2 = QUrl::fromLocalFile("/tmp/2.txt");
        model->d->fileList.append(in1);
        model->d->fileList.append(in2);

        stub.set_lamda(&FileInfoModelShell::index, [this](FileInfoModelShell *, const QUrl &url, int column){
            return model->index(url);
        });

        DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
        DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
        model->d->fileMap.insert(in1, info1);
        model->d->fileMap.insert(in2, info2);
    }
    void TearDown() override {
        delete view;
        delete model;
    }
    CollectionModel *model;
    CollectionView *view;
    CollectionItemDelegate *dlgt;
    TestSourceModel srcModel;
    FileInfoModelShell shell;
    stub_ext::StubExt stub;
};

}

TEST_F(TestCollectionItemDelegate, createEditor)
{
    stub_ext::StubExt stub;
    bool dlfs = false;
    bool trans = false;
    stub.set_lamda(&DeviceUtils::isSubpathOfDlnfs, [&dlfs](){
        return dlfs;
    });
    stub.set_lamda(&CollectionItemDelegate::isTransparent, [&trans](){
        return trans;
    });

    auto wid = dlgt->createEditor(nullptr, {}, {});
    auto editor = dynamic_cast<ItemEditor *>(wid);
    ASSERT_NE(editor, nullptr);
    EXPECT_FALSE(editor->useCharCount);
    EXPECT_EQ(editor->opacityEffect, nullptr);

    delete wid;
    dlfs = true;
    trans = true;

    wid = dlgt->createEditor(nullptr, {}, {});
    editor = dynamic_cast<ItemEditor *>(wid);
    ASSERT_NE(editor, nullptr);
    EXPECT_TRUE(editor->useCharCount);
    ASSERT_NE(editor->opacityEffect, nullptr);
    delete wid;
}

TEST_F(TestCollectionItemDelegate, updateEditorGeometry)
{
    ItemEditor editor;
    view->setFixedSize(1920, 1080);
    view->setIconSize(QSize(48, 48));

    stub_ext::StubExt stub;
    bool setbase = false;
    dlgt->d->itemSizeHint = QSize(200, 200);
    stub.set_lamda(&ItemEditor::setBaseGeometry, [&setbase](ItemEditor *, const QRect &base, const QSize &itemSize, const QMargins &margin){
        setbase = true;
        EXPECT_EQ(itemSize, QSize(200, 200));
        EXPECT_EQ(margin, QMargins(0, 53, 0, 0));
        EXPECT_EQ(base, QRect(202,202,196,196));
    });

    QStyleOptionViewItem opt;
    opt.rect = QRect(200, 200, 200, 200);
    dlgt->updateEditorGeometry(&editor, opt, QModelIndex());
    EXPECT_EQ(editor.maxHeight, 1080 - 202);
    EXPECT_TRUE(setbase);
}

TEST_F(TestCollectionItemDelegate, initStyleOption)
{
    QStyleOptionViewItem opt;
    stub_ext::StubExt stub;
    bool selected = false;
    stub.set_lamda(&QItemSelectionModel::isSelected, [&selected](){
        return selected;
    });

    bool trans = false;
    stub.set_lamda(&CollectionItemDelegate::isTransparent, [&trans](){
        return trans;
    });
    dlgt->initStyleOption(&opt, model->index(0, 0));
    EXPECT_FALSE(opt.state & QStyle::State_Selected);
    EXPECT_EQ(opt.backgroundBrush, QBrush(QColor("#2da6f7")));
    EXPECT_EQ(opt.palette.color(QPalette::BrightText), Qt::white);
    EXPECT_EQ(opt.palette.brush(QPalette::Shadow), QBrush(QColor(0, 0, 0, 178)));
    EXPECT_EQ(opt.palette.brush(QPalette::Text), QBrush(Qt::white));

    selected = true;
    trans = true;
    dlgt->initStyleOption(&opt, model->index(0, 0));
    EXPECT_TRUE(opt.state & QStyle::State_Selected);
    EXPECT_EQ(opt.backgroundBrush, QBrush(QColor("#BFE4FC")));
}

TEST_F(TestCollectionItemDelegate, mayExpand)
{
    QModelIndexList sel;
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(QAbstractItemView,selectedIndexes), [&sel](){
        return sel;
    });

    QModelIndex idx;
    EXPECT_FALSE(dlgt->mayExpand(&idx));
    EXPECT_FALSE(idx.isValid());

    idx = QModelIndex();
    sel.append(model->index(0, 0));
    EXPECT_TRUE(dlgt->mayExpand(&idx));
    EXPECT_EQ(idx, model->index(0, 0));

    idx = QModelIndex();
    sel.append(model->index(1, 0));
    EXPECT_FALSE(dlgt->mayExpand(&idx));
    EXPECT_FALSE(idx.isValid());
}

TEST_F(TestCollectionItemDelegate, paintIcon)
{
    stub_ext::StubExt stub;
    bool paint = false;
    stub.set_lamda(&CollectionItemDelegate::getIconPixmap, [&paint](const QIcon &icon, const QSize &size,
                   qreal pixelRatio, QIcon::Mode mode, QIcon::State state){
        paint = true;
        EXPECT_EQ(mode, QIcon::Normal);
        EXPECT_EQ(state, QIcon::Off);
        EXPECT_EQ(pixelRatio, 1.25);
        EXPECT_EQ(size, QSize(100, 100));
        QPixmap pix(size * pixelRatio);
        pix.fill(Qt::gray);
        pix.setDevicePixelRatio(pixelRatio);
        return pix;
    });

    QPixmap pix(125, 125);
    pix.setDevicePixelRatio(1.25);
    QPainter pa(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    auto retSize = dlgt->paintDragIcon(&pa, option, model->index(0, 0));
    EXPECT_EQ(retSize, QSize(100, 100));
    EXPECT_TRUE(paint);
}

TEST_F(TestCollectionItemDelegate, drawNormlText)
{
    QPixmap pix(125, 125);
    pix.setDevicePixelRatio(1.25);
    QPainter pa(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.textElideMode = Qt::ElideMiddle;

    stub_ext::StubExt stub;
    bool paint = false;
    stub.set_lamda(&ElideTextLayout::layout, [&paint](ElideTextLayout *,
                   const QRectF &rect, Qt::TextElideMode elideMode, QPainter *painter, const QBrush &background, QStringList *textLines){
        paint = true;
        EXPECT_EQ(rect, QRectF(0,0, 100, 100));
        EXPECT_EQ(elideMode, Qt::ElideMiddle);
        EXPECT_EQ(background, QBrush());
        return QList<QRectF>();
    });

    dlgt->drawNormlText(&pa, option, model->index(0, 0), option.rect);
    EXPECT_TRUE(paint);
}

TEST_F(TestCollectionItemDelegate, drawExpandText)
{
    QPixmap pix(125, 125);
    pix.setDevicePixelRatio(1.25);
    QPainter pa(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.palette.setColor(QPalette::Normal, QPalette::Highlight, Qt::yellow);
    option.palette.setColor(QPalette::BrightText, Qt::cyan);
    option.textElideMode = Qt::ElideRight;

    stub_ext::StubExt stub;
    bool paint = false;
    stub.set_lamda(&ElideTextLayout::layout, [&paint](ElideTextLayout *lay,
                   const QRectF &rect, Qt::TextElideMode elideMode, QPainter *painter, const QBrush &background, QStringList *textLines){
        paint = true;
        EXPECT_EQ(rect, QRectF(0, 0, 100, 100));
        EXPECT_EQ(elideMode, Qt::ElideRight);
        EXPECT_EQ(background, QBrush(Qt::yellow));
        EXPECT_EQ(lay->attribute<int>(ElideTextLayout::kBackgroundRadius), 4);
        EXPECT_EQ(painter->pen().color(), QColor(Qt::cyan));
        return QList<QRectF>();
    });

    dlgt->drawExpandText(&pa, option, model->index(0, 0), option.rect);
    EXPECT_TRUE(paint);
}
