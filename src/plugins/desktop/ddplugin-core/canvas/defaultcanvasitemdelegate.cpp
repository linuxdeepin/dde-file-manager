/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
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
#include "private/defaultcanvasitemdelegate_p.h"
#include "private/defaultcanvasview_p.h"
#include "defaultcanvasmodel.h"

#include <dtkcore_global.h>
#include <private/qtextengine_p.h>

#include <QStyledItemDelegate>
#include <QEvent>
#include <QPainter>
#include <QTextLayout>
#include <QTextBlock>
#include <QAbstractItemView>
#include <DApplication>
#include <DApplicationHelper>
#include <cmath>

#define TEXT_PADDING 4
#define ICON_MODE_ICON_SPACING 5
#define ICON_SPACING 16
#define ICON_MODE_BACK_RADIUS 18
#define ICON_MODE_RECT_RADIUS 4
#define CEIL(x) (static_cast<int>(std::ceil(x)))
#define FLOOR(x) (static_cast<int>(std::floor(x)))

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

bool ExpandedItem::event(QEvent *ee)
{
    if (ee->type() == QEvent::DeferredDelete) {
        if (!canDeferredDelete) {
            ee->accept();
            return true;
        }
    }
    return QWidget::event(ee);
}

qreal ExpandedItem::opacity() const
{
    return m_opactity;
}

void ExpandedItem::setOpacity(qreal opacity)
{
    if (qFuzzyCompare(opacity, m_opactity))
        return;
    m_opactity = opacity;
    update();
}

void ExpandedItem::paintEvent(QPaintEvent *)
{
}

QSize ExpandedItem::sizeHint() const
{
    return QSize();
}

int ExpandedItem::heightForWidth(int width) const
{
    if (width != this->width()) {
        textBounding = QRect();
    }
    return 1;
}

void ExpandedItem::setIconPixmap(const QPixmap &pixmap, int height)
{
    iconPixmap = pixmap;
    iconHeight = height;
    update();
}

QRectF ExpandedItem::iconGeometry() const
{
    const QRect &content_rect = contentsRect();
    if (iconPixmap.isNull()) {
        QRectF rect(content_rect);
        rect.setHeight(iconHeight);
        return rect;
    }
    QRectF iconRect(QPointF((content_rect.width() - iconPixmap.width() / iconPixmap.devicePixelRatio()) / 2.0,
                             (iconHeight - iconPixmap.height() / iconPixmap.devicePixelRatio()) / 2.0 + content_rect.top()),
                     iconPixmap.size() / iconPixmap.devicePixelRatio());
    return iconRect;
}

QRectF ExpandedItem::textGeometry(int width) const
{
    Q_UNUSED(width)
    return textBounding;
}

DefaultCanvasItemDelegate::DefaultCanvasItemDelegate(dfmbase::AbstractCanvas *parent)
    : dfmbase::AbstractCanvasDelegate(parent), d(new DefaultCanvasItemDelegatePrivate(this))
{
    // 临时放于此处
    d->textLineHeight = this->parent()->fontMetrics().lineSpacing();
}

DefaultCanvasItemDelegate::~DefaultCanvasItemDelegate()
{
}

/*!
 * \brief 获取当前图标等级
 * \return 返回当前图标等级
 */
int DefaultCanvasItemDelegate::currentIconSizeLevel() const
{
    // 暂时固定为： 32 << 48 << 64 << 96 << 128，后续开设注册接口
    return d->currentIconSizeIndex;
}

/*!
 * \brief 通过图标等级获取图标大小
 * \param  \a lv 图标等级
 * \return 返回对应等级图标大小
 */
QSize DefaultCanvasItemDelegate::getIconSizeByIconSizeLevel(const int lv) const
{
    int size = d->iconSizes.at(lv);
    return QSize(size, size);
}

QSize DefaultCanvasItemDelegate::getCurrentIconSize() const
{
    int size = d->iconSizes.at(d->currentIconSizeIndex);
    return QSize(size, size);
}

/*!
 * \brief 通过图标等级设置图标大小
 * \param \a lv 图标等级
 * \return 成功返回设置的图标等级大小，失败返回-1
 */
int DefaultCanvasItemDelegate::setIconSizeByIconSizeLevel(const int lv)
{
    if (lv == d->currentIconSizeIndex) {
        return lv;
    }

    if (lv >= minimumIconSizeLevel() && lv <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = lv;
        parent()->setIconSize(getCurrentIconSize());
        return d->currentIconSizeIndex;
    }

    return -1;
}

int DefaultCanvasItemDelegate::minimumIconSizeLevel() const
{
    return 0;
}

int DefaultCanvasItemDelegate::maximumIconSizeLevel() const
{
    return d->iconSizes.count() - 1;
}

void DefaultCanvasItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 项是否可用，用于图标绘制disable与否
    bool isEnabled = option.state & QStyle::State_Enabled;
    // 焦点与否，主要用于控制字体背景显示
    bool hasFocus = option.state & QStyle::State_HasFocus;
    Q_UNUSED(hasFocus)
    // 项是否处于拖拽状态
    // judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = (static_cast<QPaintDevice *>(parent()->viewport()) != painter->device());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    DPalette pl(DApplicationHelper::instance()->palette(option.widget));
    QColor c = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    QColor baseColor = c;
    if (option.widget) {
        DPalette pa = DApplicationHelper::instance()->palette(option.widget);
        baseColor = option.widget->palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
        }
    }

    // icon
    QRectF rect = opt.rect;
    QPainterPath path;
    rect.moveTopLeft(QPointF(0.5, 0.5) + rect.topLeft());
    path.addRoundedRect(rect, ICON_MODE_BACK_RADIUS, ICON_MODE_BACK_RADIUS);

    /// init icon geomerty
    QRectF iconRect = opt.rect;

    iconRect.setSize(parent()->iconSize());
    iconRect.moveLeft(opt.rect.left() + (opt.rect.width() - iconRect.width()) / 2.0);
    iconRect.moveTop(opt.rect.top());   // move icon down
    // draw icon
    // todo: select and drop status
    const_cast<DefaultCanvasItemDelegate *>(this)->drawIcon(painter, opt.icon,
                                                            iconRect, Qt::AlignCenter,
                                                            isEnabled ? QIcon::Normal : QIcon::Disabled);
    // todo draw file additional icon (角标)

    // text
    if (index == d->editingIndex && !isDragMode) {
        // 正在编辑的item，不重绘text
        return;
    }
    if (index == d->expandedIndex && !isDragMode
        && d->expandedItem && d->expandedItem->index == index
        && d->expandedItem->option.rect == opt.rect) {
        d->expandedItem->option = opt;
        return;
    }

    QString str = parent()->fileDisplayNameRole(index);
    QRectF label_rect = opt.rect;
    label_rect.setTop(iconRect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);
    label_rect.setWidth(opt.rect.width() - 2 * TEXT_PADDING);
    label_rect.moveLeft(label_rect.left() + TEXT_PADDING);
    label_rect.setBottom(path.boundingRect().toRect().bottom());
    painter->setPen(opt.palette.color(QPalette::BrightText));
    qreal pixel_ratio = painter->device()->devicePixelRatioF();
    QImage text_image((label_rect.size() * pixel_ratio).toSize(), QImage::Format_ARGB32_Premultiplied);
    text_image.fill(Qt::transparent);
    text_image.setDevicePixelRatio(pixel_ratio);

    QPainter p(&text_image);
    p.setPen(painter->pen());
    p.setFont(painter->font());
    drawText(index, &p, str, QRectF(QPoint(0, 0), QSizeF(text_image.size()) / pixel_ratio),
             ICON_MODE_RECT_RADIUS, QBrush(Qt::NoBrush),
             QTextOption::WrapAtWordBoundaryOrAnywhere, opt.textElideMode, Qt::AlignCenter);
    p.end();

    QPixmap text_pixmap = QPixmap::fromImage(text_image);
    text_pixmap.setDevicePixelRatio(pixel_ratio);
    qt_blurImage(text_image, 6, false);

    p.begin(&text_image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(text_image.rect(), opt.palette.color(QPalette::Shadow));
    p.end();

    painter->drawImage(label_rect.translated(0, 1), text_image);
    painter->drawPixmap(label_rect.topLeft(), text_pixmap);
    painter->setOpacity(1);
}

QList<QRectF> DefaultCanvasItemDelegate::drawText(const QModelIndex &index, QPainter *painter, QTextLayout *layout,
                                                  const QRectF &boundingRect, qreal radius, const QBrush &background,
                                                  QTextOption::WrapMode wordWrap, Qt::TextElideMode mode, int flags,
                                                  const QColor &shadowColor) const
{
    initTextLayout(index, layout);
    QList<QRectF> boundingRegion;
    AbstractCanvasDelegate::elideText(layout, boundingRect.size(), wordWrap, mode, d->textLineHeight, flags, nullptr,
                                      painter, boundingRect.topLeft(), shadowColor, QPointF(0, 1),
                                      background, radius, &boundingRegion);
    return boundingRegion;
}

QList<QRectF> DefaultCanvasItemDelegate::drawText(const QModelIndex &index, QPainter *painter, const QString &text,
                                                  const QRectF &boundingRect, qreal radius, const QBrush &background,
                                                  QTextOption::WrapMode wordWrap, Qt::TextElideMode mode, int flags,
                                                  const QColor &shadowColor) const
{
    QTextLayout layout;
    layout.setText(text);
    if (painter)
        layout.setFont(painter->font());
    return drawText(index, painter, &layout, boundingRect, radius, background, wordWrap, mode, flags, shadowColor);
}

DefaultCanvasView *DefaultCanvasItemDelegate::parent() const
{
    return dynamic_cast<DefaultCanvasView *>(QObject::parent());
}

/*!
 * \brief 返回所有正显示部件的索引
 * \return 显示部件的索引
 */
QModelIndexList DefaultCanvasItemDelegate::hasWidgetIndexs() const
{
    // todo:
    return QModelIndexList();
}

/*!
 * \brief 隐藏未编辑的索引对应的部件
 */
void DefaultCanvasItemDelegate::hideNotEditingIndexWidget()
{
    if (d->expandedIndex.isValid()) {
        parent()->setIndexWidget(d->expandedIndex, nullptr);
        d->expandedItem->hide();
        d->expandedIndex = QModelIndex();
        d->lastAndExpandedIndex = QModelIndex();
    }
}

/*!
 * \brief 返回扩展项的索引
 * \return 扩展项索引
 */
QModelIndex DefaultCanvasItemDelegate::expandedIndex() const
{
    return d->editingIndex;
}

/*!
 * \brief 返回扩展索引项部件
 * \return 所在索引项部件
 */
QWidget *DefaultCanvasItemDelegate::expandedIndexWidget() const
{
    return d->expandedItem;
}

bool DefaultCanvasItemDelegate::isTransparent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    // todo 剪切等半透明状态绘制判断
    return false;
}

Qt::Alignment DefaultCanvasItemDelegate::visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
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

QPixmap DefaultCanvasItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    bool useHighDpiPixmaps = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, false);

    if (icon.isNull())
        return QPixmap();
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    QSize icon_size = icon.actualSize(size, mode, state);
    QList<QSize> iconSizeList = icon.availableSizes();
    QSize iconRealSize;
    if (iconSizeList.count() > 0)
        iconRealSize = iconSizeList.first();
    else
        iconRealSize = icon_size;
    if (iconRealSize.width() <= 0 || iconRealSize.height() <= 0)
        return icon.pixmap(icon_size);

    bool isSpecialSize = false;
    QSize tempSize(size.width(), size.height());
    while (icon_size.width() < 1) {
        tempSize.setHeight(tempSize.height() * 2);
        icon_size = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }
    while (icon_size.height() < 1) {
        tempSize.setWidth(tempSize.width() * 2);
        icon_size = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }

    if ((icon_size.width() > size.width() || icon_size.height() > size.height()) && !isSpecialSize)
        icon_size.scale(size, Qt::KeepAspectRatio);

    QSize pixmapSize = icon_size * pixelRatio;
    QPixmap px = icon.pixmap(pixmapSize, mode, state);

    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, useHighDpiPixmaps);

    if (isSpecialSize) {
        if (px.width() > size.width() * pixelRatio) {
            px = px.scaled(size.width() * CEIL(pixelRatio), px.height(), Qt::IgnoreAspectRatio);
        } else if (px.height() > size.height() * pixelRatio) {
            px = px.scaled(px.width(), size.height() * CEIL(pixelRatio), Qt::IgnoreAspectRatio);
        }
    }

    // 类型限定符的更改会导致缩放小数点丢失，从而引发缩放因子的bug
    if (px.width() > icon_size.width() * pixelRatio) {
        px.setDevicePixelRatio(px.width() / qreal(icon_size.width()));
    } else if (px.height() > icon_size.height() * pixelRatio) {
        px.setDevicePixelRatio(px.height() / qreal(icon_size.height()));
    } else {
        px.setDevicePixelRatio(pixelRatio);
    }

    return px;
}

QList<QRectF> DefaultCanvasItemDelegate::getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const
{
    QList<QRectF> list;
    double offset = baseRect.width() / 8;
    const QSizeF &offset_size = cornerSize / 2;

    list.append(QRectF(QPointF(baseRect.right() - offset - offset_size.width(),
                               baseRect.bottom() - offset - offset_size.height()),
                       cornerSize));
    list.append(QRectF(QPointF(baseRect.left() + offset - offset_size.width(), list.first().top()), cornerSize));
    list.append(QRectF(QPointF(list.at(1).left(), baseRect.top() + offset - offset_size.height()), cornerSize));
    list.append(QRectF(QPointF(list.first().left(), list.at(2).top()), cornerSize));

    return list;
}

QRectF DefaultCanvasItemDelegate::initIconGeometry(const QStyleOptionViewItem &opt) const
{
    QRectF iconRect = opt.rect;
    iconRect.setSize(parent()->iconSize());
    iconRect.moveLeft(opt.rect.left() + (opt.rect.width() - iconRect.width()) / 2.0);
    iconRect.moveTop(opt.rect.top());
    return iconRect;
}

void DefaultCanvasItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    Q_UNUSED(layout)
    const QVariantHash &ep = index.data(DefaultCanvasModel::ExtraProperties).toHash();
    const QList<QColor> &colors = qvariant_cast<QList<QColor>>(ep.value("colored"));
    Q_UNUSED(colors)
}

QColor DefaultCanvasItemDelegate::itemBaseColor(const QStyleOptionViewItem &opt, bool isSelected, bool isDropTarget) const
{
    DPalette pl(DApplicationHelper::instance()->palette(opt.widget));
    QColor tempColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    QColor baseColor = tempColor;
    if (opt.widget) {
        DPalette pa = DApplicationHelper::instance()->palette(opt.widget);
        baseColor = opt.widget->palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
        }
    }

    if ((isDropTarget && !isSelected) || opt.state & QStyle::StateFlag::State_Selected) {
        tempColor = pl.color(DPalette::ColorGroup::Active, QPalette::ColorRole::Highlight);
    } else if (opt.state & QStyle::StateFlag::State_MouseOver) {
        tempColor = tempColor.lighter();
    }
    return tempColor;
}

void DefaultCanvasItemDelegate::setIconBaseParameter(QPainter *painter, const QColor &clr, const QStyleOptionViewItem &option, bool isSelected, bool isDragMode) const
{
    QPainterPath path;
    QRectF rect = option.rect;
    rect.moveTopLeft(QPointF(0.5, 0.5) + rect.topLeft());
    path.addRoundedRect(rect, ICON_MODE_BACK_RADIUS, ICON_MODE_BACK_RADIUS);

    // 设置底色、拖拽模式下的渲染方式、圆角
    if (isDragMode && !isSelected) {
        painter->setPen(clr);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
    }
}

QRectF DefaultCanvasItemDelegate::setTextBaseParameter(QPainter *painter, const QStyleOptionViewItem &opt, const QRectF &iconRect, bool isSelected, bool isDragMode) const
{
    Q_UNUSED(isSelected)
    Q_UNUSED(isDragMode)
    QString str = opt.text;
    // init file name geometry
    QRectF labelRect = opt.rect;

    labelRect.setTop(iconRect.bottom() + 4 /*TEXT_PADDING*/ + /*ICON_MODE_ICON_SPACING*/ 5);
    labelRect.setWidth(opt.rect.width() - 2 * 4 /*TEXT_PADDING*/ - ICON_MODE_BACK_RADIUS);
    labelRect.moveLeft(labelRect.left() + 4 /*TEXT_PADDING*/ + ICON_MODE_BACK_RADIUS / 2);

    painter->setPen(opt.palette.color(QPalette::BrightText));
    return labelRect;
}

void DefaultCanvasItemDelegate::drawIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state)
{
    // Copy of QStyle::alignedRect
    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixel_ratio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, rect.size().toSize(), pixel_ratio, mode, state);
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

void DefaultCanvasItemDelegate::drawFileName(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index, const QString &str, const QRectF labelRect) const
{
    qreal pixel_ratio = painter->device()->devicePixelRatioF();
    QImage text_image((labelRect.size() * pixel_ratio).toSize(), QImage::Format_ARGB32_Premultiplied);
    text_image.fill(Qt::transparent);
    text_image.setDevicePixelRatio(pixel_ratio);

    QPainter p(&text_image);
    p.setPen(painter->pen());
    p.setFont(painter->font());
    drawText(index, &p, str, QRectF(QPoint(0, 0), QSizeF(text_image.size()) / pixel_ratio),
             ICON_MODE_RECT_RADIUS, QBrush(Qt::NoBrush),
             QTextOption::WrapAtWordBoundaryOrAnywhere, opt.textElideMode, Qt::AlignCenter);
    p.end();

    QPixmap text_pixmap = QPixmap::fromImage(text_image);
    text_pixmap.setDevicePixelRatio(pixel_ratio);
    qt_blurImage(text_image, 6, false);

    p.begin(&text_image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(text_image.rect(), opt.palette.color(QPalette::Shadow));
    p.end();

    painter->drawImage(labelRect.translated(0, 1), text_image);
    painter->drawPixmap(labelRect.topLeft(), text_pixmap);
}

QSize DefaultCanvasItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_UNUSED(index)
    const QSize &size = d->itemSizeHint;
    if (index.isValid() && index == d->lastAndExpandedIndex) {
        d->expandedItem->iconHeight = parent()->iconSize().height();
        return QSize(size.width(), d->expandedItem->heightForWidth(size.width()));
    }
    return size;
}

void DefaultCanvasItemDelegate::updateItemSizeHint()
{
    // todo 处理一下这些魔数
    int width = parent()->iconSize().width() * 17 / 10;
    int height = parent()->iconSize().height()
            + 10 + 2 * d->textLineHeight;
    d->itemSizeHint = QSize(width, height);
}
