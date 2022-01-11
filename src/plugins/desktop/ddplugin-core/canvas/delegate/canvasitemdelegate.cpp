/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "canvasitemdelegate_p.h"
#include "elidetextlayout.h"
#include "view/canvasview.h"
#include "view/canvasmodel.h"
#include "view/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "filetreater.h"

#include <DApplication>
#include <DApplicationHelper>

#include <QPainter>
#include <QAbstractTextDocumentLayout>

#include <private/qtextengine_p.h>

#include <cmath>

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE


DWIDGET_USE_NAMESPACE
DSB_D_BEGIN_NAMESPACE

const int CanvasItemDelegate::kTextPadding = 4;
const int CanvasItemDelegate::kIconSpacing =  5;
const int CanvasItemDelegate::kTconBackRadius = 18;
const int CanvasItemDelegate::kIconRectRadius = 4;

int CanvasItemDelegatePrivate::textObjectType = QTextFormat::UserObject + 1;
FileTagObjectInterface *CanvasItemDelegatePrivate::textObjectInterface = new FileTagObjectInterface();

CanvasItemDelegatePrivate::CanvasItemDelegatePrivate(CanvasItemDelegate *qq)
    : q(qq)
{

}

CanvasItemDelegatePrivate::~CanvasItemDelegatePrivate()
{

}

ElideTextLayout *CanvasItemDelegatePrivate::createTextlayout(const QModelIndex &index, const QPainter *painter) const
{
    ElideTextLayout *layout = new ElideTextLayout(index.data(FileTreater::kFileDisplayNameRole).toString());

    // tag rect
    q->initTextLayout(index, layout);

    layout->setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout->setLineHeight(textLineHeight);
    layout->setAlignment(Qt::AlignCenter);

    if (painter) {
        layout->setFont(painter->font());
        layout->setTextDirection(painter->layoutDirection());
    }

    return layout;
}

CanvasItemDelegate::CanvasItemDelegate(QAbstractItemView *parentPtr)
    : QStyledItemDelegate(parentPtr)
    , d(new CanvasItemDelegatePrivate(this))
{
    // 初始化图标等级、大小信息
    d->iconSizes << 32 << 48 << 64 << 96 << 128;
    d->iconLevelDescriptions << tr("Tiny")
                         << tr("Small")
                         << tr("Medium")
                         << tr("Large")
                         << tr("Super large");


    // 初始化默认图标为小
    const int iconLevel = 1;
    Q_ASSERT(iconLevel < d->iconSizes.size());
    setIconLevel(iconLevel);

    // todo(zy) need?
//    d->expandedItem = new ExpandedItem(this, parent()->viewport());
//    d->expandedItem->setAttribute(Qt::WA_TransparentForMouseEvents);
//    d->expandedItem->canDeferredDelete = false;
//    d->expandedItem->setContentsMargins(0, 0, 0, 0);
//    // prevent flash when first call show()
//    d->expandedItem->setFixedWidth(0);
    d->textLineHeight = parent()->fontMetrics().height();


    // TODO:  model rowsInserted and iconSizeChanged etc
}

CanvasItemDelegate::~CanvasItemDelegate()
{

}

QSize CanvasItemDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    Q_UNUSED(opt)
    const QSize &size = d->itemSizeHint;
    // todo(zy) for what
//    if (index.isValid() && index == d->lastExpandedIndex) {
//        d->expandedItem->iconHeight = parent()->iconSize().height();
//        return QSize(size.width(), d->expandedItem->heightForWidth(size.width()));
//    }

    return size;
}

void CanvasItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // the global option.
    QStyleOptionViewItem indexOption = option;

    // init option for each index.
    // and the rect of index was inited outside.
    initStyleOption(&indexOption, index);

    painter->save();

    // paint a translucent effect.
    painter->setOpacity(parent()->isTransparent(index) ? 0.3 : 1.0);

    // get item paint geomerty
    // the method to get rect for each element is equal to paintGeomertys(option, index);
    {
        // draw icon
        const QRect rIcon = iconRect(option.rect);
        paintIcon(painter, indexOption.icon, rIcon, Qt::AlignCenter,
                  (option.state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled); //why Enabled?

        // todo(zy) 绘制角标

        // draw text.
         paintLabel(painter, indexOption, index, labelRect(option.rect, rIcon));
    }

    painter->restore();
}

bool CanvasItemDelegate::mayExpand(QModelIndex *who) const
{
    QModelIndexList list = parent()->selectionModel()->selectedIndexes();
    bool expend = list.size() == 1;
    if (expend && who)
        *who = list.first();

    return expend;
}

QRectF CanvasItemDelegate::boundingRect(const QList<QRectF> &rects)
{
    QRectF bounding;

    if (rects.isEmpty())
        return bounding;

    bounding = rects.first();

    for (const QRectF &r : rects) {
        if (r.top() < bounding.top()) {
            bounding.setTop(r.top());
        }

        if (r.left() < bounding.left()) {
            bounding.setLeft(r.left());
        }

        if (r.right() > bounding.right()) {
            bounding.setRight(r.right());
        }

        if (r.bottom() > bounding.bottom()) {
            bounding.setBottom(r.bottom());
        }
    }

    return bounding;
}

QList<QRect> CanvasItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<QRect> geometries;
    // todo(zy) index == d->expandedIndex

    // icon rect, for draw icon
    auto icon = iconRect(option.rect);
    geometries << icon;

    // label rect is a hot zone and contain text rect.
    auto label = labelRect(option.rect, icon);
    geometries << label;

    // calc text rect base on label. and text rect is for draw text.
    auto text = textPaintRect(option, index, label);
    geometries << text;

    return geometries;
}

Qt::Alignment CanvasItemDelegate::visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
{
    if (!(alignment & Qt::AlignHorizontal_Mask))
        alignment |= Qt::AlignLeft;
    if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
        if (direction == Qt::RightToLeft)
            alignment ^= (Qt::AlignLeft | Qt::AlignRight);
        alignment |= Qt::AlignAbsolute;
    }
    return alignment;
}

QList<QRectF> CanvasItemDelegate::elideTextRect(const QStyleOptionViewItem &option, const QModelIndex &index, QRect rect) const
{
    // show decoration for selected item.
    auto model = parent()->selectionModel();
    bool isSelected = model->isSelected(index) && option.showDecorationSelected;

    QModelIndex expandIndex;
    // only single selected no need to elide. and other item need elide.
    bool elide = (!isSelected || !mayExpand(&expandIndex));
    if (!elide && expandIndex == index) // as painting, the item expanded need unlimit height to calc.
        rect.setBottom(INT_MAX);

    // create text Layout.
    QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index));

    // elide mode
    auto elideMode = elide ? option.textElideMode : Qt::ElideNone;
    auto textLines = layout->layout(rect, elideMode);
    return textLines;
}

void CanvasItemDelegate::drawNormlText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const
{
    painter->save();
    painter->setPen(option.palette.color(QPalette::Text));

    qreal pixelRatio = painter->device()->devicePixelRatioF();
    QImage textImage((rect.size() * pixelRatio).toSize(), QImage::Format_ARGB32_Premultiplied);
    textImage.fill(Qt::transparent);
    textImage.setDevicePixelRatio(pixelRatio);

    QPixmap textPixmap;
    {
        QPainter p(&textImage);
        p.setPen(painter->pen());
        p.setFont(painter->font());

        // create text Layout.
        QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index, &p));

        // elide and draw
        layout->layout(QRectF(QPoint(0, 0), QSizeF(textImage.size()) / pixelRatio), option.textElideMode, &p);
        p.end();

        textPixmap = QPixmap::fromImage(textImage);
        textPixmap.setDevicePixelRatio(pixelRatio);
        qt_blurImage(textImage, 6, false);

        p.begin(&textImage);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(textImage.rect(), option.palette.color(QPalette::Shadow));
        p.end();
    }

    painter->drawImage(rect.translated(0, 1), textImage);
    painter->drawPixmap(rect.topLeft(), textPixmap);
    painter->restore();
}

void CanvasItemDelegate::drawHighlightText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const
{
    bool isDrag = painter->device() != parent()->viewport(); // 拖拽聚合后，拖拽的绘制代码不走这，可以删除， todo

    // single item selected and not in drag will to expand.
    if (!isDrag && mayExpand()) {
        // calc that showing text require how large area.
        QRect calcNeedRect = rect.toRect();
        calcNeedRect.setBottom(INT_MAX);
        int needHeight = textPaintRect(option, index, calcNeedRect).height();

        // the label rect cannot show all the text, need to expand.
        if (needHeight > rect.height()) {
            {
                // 展开时文本区域的宽需扩大到整个网格
                // 处理会导致实际绘制使用的区域与paintGeomertys以及textPaintRect算出的不一致
                // todo 后续考虑是否取消这一效果
                auto margins = CanvasViewPrivate::gridMarginsHelper(parent());
                margins.setTop(0);
                margins.setBottom(0);
                calcNeedRect.moveLeft(calcNeedRect.left() - kTextPadding);
                calcNeedRect.setWidth(calcNeedRect.width() + 2 * kTextPadding);
                calcNeedRect = calcNeedRect.marginsAdded(margins);
            }

            drawExpandText(painter, option, index, calcNeedRect);
            return;
        }
    }

    {
        painter->save();
        painter->setPen(option.palette.color(QPalette::BrightText));
        auto background = option.palette.brush(QPalette::Normal, QPalette::Highlight);

        // create text Layout.
        QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index, painter));
        layout->setBackgroundRadius(kIconRectRadius);

        // elide and draw
        layout->layout(rect, option.textElideMode, painter, background);
        painter->restore();
    }
}

void CanvasItemDelegate::drawExpandText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const
{
    painter->save();
    painter->setPen(option.palette.color(QPalette::BrightText));
    auto background = option.palette.brush(QPalette::Normal, QPalette::Highlight);

    // create text Layout.
    QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index, painter));
    layout->setBackgroundRadius(kIconRectRadius);

    // elide and draw
    layout->layout(rect, option.textElideMode, painter, background);
    painter->restore();
}

QPixmap CanvasItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size,
                                          qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    // TODO: 优化

    if (icon.isNull())
        return QPixmap();

    // 确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    // 开启Qt::AA_UseHighDpiPixmaps后，QIcon::pixmap会自动执行 pixmapSize *= qApp->devicePixelRatio()
    // 而且，在有些QIconEngine的实现中，会去调用另一个QIcon::pixmap，导致 pixmapSize 在这种嵌套调用中越来越大
    // 最终会获取到一个是期望大小几倍的图片，由于图片太大，会很快将 QPixmapCache 塞满，导致后面再调用QIcon::pixmap
    // 读取新的图片时无法缓存，非常影响图片绘制性能。此处在获取图片前禁用 Qt::AA_UseHighDpiPixmaps，自行处理图片大小问题
    bool useHighDpiPixmaps = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, false);

    QSize iconSize = icon.actualSize(size, mode, state);
    // 取出icon的真实大小
    QList<QSize> iconSizeList = icon.availableSizes();
    QSize iconRealSize;
    if (iconSizeList.count() > 0)
        iconRealSize = iconSizeList.first();
    else
        iconRealSize = iconSize;
    if (iconRealSize.width() <= 0 || iconRealSize.height() <= 0) {
        // restore the value
        qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, useHighDpiPixmaps);
        return icon.pixmap(iconSize);
    }

    // 确保特殊比例icon的高或宽不为0
    bool isSpecialSize = false;
    QSize tempSize(size.width(), size.height());
    while (iconSize.width() < 1) {
        tempSize.setHeight(tempSize.height() * 2);
        iconSize = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }
    while (iconSize.height() < 1) {
        tempSize.setWidth(tempSize.width() * 2);
        iconSize = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }

    if ((iconSize.width() > size.width() || iconSize.height() > size.height()) && !isSpecialSize)
        iconSize.scale(size, Qt::KeepAspectRatio);

    QSize pixmapSize = iconSize * pixelRatio;
    QPixmap px = icon.pixmap(pixmapSize, mode, state);

    // restore the value
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, useHighDpiPixmaps);

    // 约束特殊比例icon的尺寸
    if (isSpecialSize) {
        auto ceilPixelRatio = static_cast<int>(std::ceil(pixelRatio));
        if (px.width() > size.width() * pixelRatio) {
            px = px.scaled(size.width() * ceilPixelRatio, px.height(), Qt::IgnoreAspectRatio);
        } else if (px.height() > size.height() * pixelRatio) {
            px = px.scaled(px.width(), size.height() * ceilPixelRatio, Qt::IgnoreAspectRatio);
        }
    }

    // 类型限定符的更改会导致缩放小数点丢失，从而引发缩放因子的bug
    if (px.width() > iconSize.width() * pixelRatio) {
        px.setDevicePixelRatio(px.width() / qreal(iconSize.width()));
    } else if (px.height() > iconSize.height() * pixelRatio) {
        px.setDevicePixelRatio(px.height() / qreal(iconSize.height()));
    } else {
        px.setDevicePixelRatio(pixelRatio);
    }

    return px;
}

CanvasView *CanvasItemDelegate::parent() const
{
    return dynamic_cast<CanvasView *>(QObject::parent());
}

QSize CanvasItemDelegate::iconSize(int lv) const
{
    if (lv >= minimumIconLevel() && lv <= maximumIconLevel()) {
        int size = d->iconSizes.at(lv);
        return QSize(size, size);
    }

    return QSize();
}

int CanvasItemDelegate::iconLevel() const
{
    return d->currentIconLevel;
}

int CanvasItemDelegate::setIconLevel(int lv)
{
    if (lv == d->currentIconLevel)
        return lv;

    if (lv >= minimumIconLevel() && lv <= maximumIconLevel()) {
        d->currentIconLevel = lv;
        parent()->setIconSize(iconSize(lv));
        return lv;
    }

    return -1;
}

int CanvasItemDelegate::minimumIconLevel() const
{
    return 0;
}

int CanvasItemDelegate::maximumIconLevel() const
{
    return d->iconSizes.count() - 1;
}

QRect CanvasItemDelegate::iconRect(const QRect &paintRect) const
{
    QRect rIcon = paintRect;

    rIcon.setSize(parent()->iconSize());
    rIcon.moveCenter(paintRect.center());
    rIcon.moveTop(paintRect.top());

    return rIcon;
}

QRect CanvasItemDelegate::labelRect(const QRect &paintRect, const QRect &usedRect)
{
    QRect lable = paintRect;
    // label rect is under the icon.
    lable.setTop(usedRect.bottom());

    // minus text padding at left and right.
    lable.setWidth(paintRect.width() - 2 * kTextPadding);

    // move left begin text padding.
    lable.moveLeft(lable.left() + kTextPadding);

    return lable;
}

QRect CanvasItemDelegate::textPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &label) const
{
    QRect rect = d->availableTextRect(label);

    // per line
    auto lines = elideTextRect(option, index, rect);

    // total rect
    rect = boundingRect(lines).toRect();
    return rect;
}


void CanvasItemDelegate::updateItemSizeHint() const
{
    // TODO (LQ) why--> 17 10
    int width = parent()->iconSize().width() * 17 / 10;
    int height = parent()->iconSize().height()
            + 10 + 2 * d->textLineHeight;
    d->itemSizeHint = QSize(width, height);
}

void CanvasItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    // todo(zy) 注释
    const QVariantHash &ep = index.data(FileTreater::kExtraProperties).toHash();
    const QList<QColor> &colors = qvariant_cast<QList<QColor>>(ep.value("colored"));

    if (!colors.isEmpty()) {
        if (!layout->engine()->block.docHandle()) {
            if (!d->document)
                const_cast<CanvasItemDelegatePrivate *>(d)->document = new QTextDocument(const_cast<CanvasItemDelegate *>(this));

            d->document->setPlainText(layout->text());
            d->document->setDefaultFont(layout->font());
            layout->engine()->block = d->document->firstBlock();
        }

        layout->engine()->docLayout()->registerHandler(d->textObjectType, d->textObjectInterface);
        QTextCursor cursor(layout->engine()->docLayout()->document());
        TagTextFormat format(d->textObjectType, colors, (d->drawTextBackgroundOnLast || colors.size() > 1) ? Qt::white : QColor(0, 0, 0, 25));

        cursor.setPosition(0);
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
    }
}

void CanvasItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    auto view = parent();
    auto model = view->selectionModel();

    // selected
    if (model->isSelected(index)) {
         // set seleted state. it didn't be seted in QStyledItemDelegate::initStyleOption.
        option->state |= QStyle::State_Selected;
    } else {
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);
    }

    // enable
    if (option->state & QStyle::State_Enabled) {
        QPalette::ColorGroup cg;
        if (view->model()->flags(index) & Qt::ItemIsEnabled) {
            cg = QPalette::Normal;
        } else { // item is not enable.
            option->state &= ~QStyle::State_Enabled;
            cg = QPalette::Disabled;
        }
        option->palette.setCurrentColorGroup(cg);
    }

    option->palette.setColor(QPalette::Text, QColor("white"));
    option->palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#797979"));

    // selected and show selected decoration need highlight
    if ((option->state & QStyle::State_Selected) && option->showDecorationSelected) {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#e9e9e9"));
    } else {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#797979"));
    }

    option->palette.setColor(QPalette::BrightText, Qt::white);
    option->palette.setBrush(QPalette::Shadow, QColor(0, 0, 0, 178));

    // multi-selected background
    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && model->selectedIndexes().size() > 1) {
        option->palette.setColor(QPalette::Background, QColor("#0076F9"));
        option->backgroundBrush = QColor("#0076F9");
    } else { // normal
        option->palette.setColor(QPalette::Background, QColor("#2da6f7"));
        option->backgroundBrush = QColor("#2da6f7");
    }

    // int cut
    if (view->isTransparent(index))
        option->backgroundBrush = QColor("#BFE4FC");

    // why?
    option->state &= ~QStyle::State_MouseOver;

    // elide mode for each file
    option->textElideMode = Qt::ElideLeft;
}

/*!
 * \brief paint icon
 * \param painter
 * \param icon: the icon to paint
 * \param rect: icon rect
 * \param alignment: alignment if icon
 * \param mode: icon mode (Normal, Disabled, Active, Selected )
 * \param state: The state for which a pixmap is intended to be used. (On, Off)
 */
void CanvasItemDelegate::paintIcon(QPainter *painter, const QIcon &icon,
                                   const QRectF &rect, Qt::Alignment alignment,
                                   QIcon::Mode mode, QIcon::State state)
{
    // Copy of QStyle::alignedRect
    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, rect.size().toSize(), pixelRatio, mode, state);
    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (rect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (rect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}

void CanvasItemDelegate::paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rect) const
{
    painter->save();

    QRect availableRect = d->availableTextRect(rect);

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected) {
        drawHighlightText(painter, option, index, availableRect);
    } else {
        drawNormlText(painter, option, index, availableRect);
    }
    painter->restore();
}



DSB_D_END_NAMESPACE
