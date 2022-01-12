/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "diconitemdelegate.h"
#include "dfileviewhelper.h"
#include "views/fileitem.h"
#include "private/dstyleditemdelegate_p.h"
#include "dfmapplication.h"
#include "dfilesystemmodel.h"
#include "tag/tagmanager.h"
#include "app/define.h"
#include "dfmglobal.h"

#include <dgiosettings.h>

#include <DApplicationHelper>
#include <DStyleOption>
#include <DStyle>
#include <DApplication>

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QPainterPath>
#include <private/qtextengine_p.h>
#include <QToolTip>

DWIDGET_USE_NAMESPACE
DFM_USE_NAMESPACE

#define ICON_SPACING 16
#define ICON_MODE_RECT_RADIUS TEXT_PADDING
#define ICON_MODE_BACK_RADIUS 18

QRectF boundingRect(const QList<QRectF> &rects)
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

QPainterPath boundingPath(QList<QRectF> rects, qreal radius, qreal padding)
{
    QPainterPath path;
    const QMarginsF margins(radius + padding, 0, radius + padding, 0);

    if (rects.count() == 1) {
        path.addRoundedRect(rects.first().marginsAdded(margins).adjusted(0, -padding, 0, padding), radius, radius);

        return path;
    }

    auto joinRightCorner = [&](const QRectF & rect, const QRectF & prevRect, const QRectF & nextRect) {
        if (Q_LIKELY(prevRect.isValid())) {
            qreal new_radius = qMin(radius, qAbs(prevRect.right() - rect.right()) / 2);

            if (rect.right() > prevRect.right()) {
                path.arcTo(rect.right() - new_radius * 2, rect.y() - padding, new_radius * 2, new_radius * 2, 90, -90);
            } else if (rect.right() < prevRect.right()) {
                path.arcTo(rect.right(), rect.y() + padding, new_radius * 2, new_radius * 2, 90, 90);
            }
        } else {
            path.arcTo(rect.right() - radius * 2, rect.y() - padding, radius * 2, radius * 2, 90, -90);
        }

        if (Q_LIKELY(nextRect.isValid())) {
            qreal new_radius = qMin(radius, qAbs(nextRect.right() - rect.right()) / 2);

            if (rect.right() > nextRect.right()) {
                path.arcTo(rect.right() - new_radius * 2, rect.bottom() - new_radius * 2 + padding, new_radius * 2, new_radius * 2, 0, -90);
            } else if (rect.right() < nextRect.right()) {
                path.arcTo(rect.right(), rect.bottom() - new_radius * 2 - padding, new_radius * 2, new_radius * 2, 180, 90);
            }
        } else {
            path.arcTo(rect.right() - radius * 2, rect.bottom() - radius * 2 + padding, radius * 2, radius * 2, 0, -90);
        }
    };

    auto joinLeftCorner = [&](const QRectF & rect, const QRectF & prevRect, const QRectF & nextRect) {
        if (Q_LIKELY(nextRect.isValid())) {
            qreal new_radius = qMin(radius, qAbs(nextRect.x() - rect.x()) / 2);

            if (rect.x() > nextRect.x()) {
                path.arcTo(rect.x() - new_radius * 2, rect.bottom() - new_radius * 2 - padding, new_radius * 2, new_radius * 2, 270, 90);
            } else if (rect.x() < nextRect.x()) {
                path.arcTo(rect.x(), rect.bottom() - new_radius * 2 + padding, new_radius * 2, new_radius * 2, 270, -90);
            }
        } else {
            path.arcTo(rect.x(), rect.bottom() - radius * 2 + padding, radius * 2, radius * 2, 270, -90);
        }

        if (Q_LIKELY(prevRect.isValid())) {
            qreal new_radius = qMin(radius, qAbs(prevRect.x() - rect.x()) / 2);

            if (rect.x() > prevRect.x()) {
                path.arcTo(rect.x() - new_radius * 2, rect.y() + padding, new_radius * 2, new_radius * 2, 0, 90);
            } else if (rect.x() < prevRect.x()) {
                path.arcTo(rect.x(), rect.y() - padding, new_radius * 2, new_radius * 2, 180, -90);
            }
        } else {
            path.arcTo(rect.x(), rect.y() - padding, radius * 2, radius * 2, 180, -90);
        }
    };

    auto preproccess = [&](QRectF & rect, const QRectF & prev) {
        if (qAbs(rect.x() - prev.x()) < radius) {
            rect.setLeft(prev.x());
        }

        if (qAbs(rect.right() - prev.right()) < radius) {
            rect.setRight(prev.right());
        }
    };

    for (int i = 1; i < rects.count(); ++i) {
        preproccess(rects[i], rects.at(i - 1));
    }

    const QRectF &first = rects.first().marginsAdded(margins);

    path.arcMoveTo(first.right() - radius * 2, first.y() - padding, radius * 2, radius * 2, 90);
    joinRightCorner(first, QRectF(), rects.at(1).marginsAdded(margins));

    for (int i = 1; i < rects.count() - 1; ++i) {
        joinRightCorner(rects.at(i) + margins, rects.at(i - 1).marginsAdded(margins), rects.at(i + 1).marginsAdded(margins));
    }

    QRectF last = rects.last();
    const QRectF &prevRect = rects.at(rects.count() - 2);

    joinRightCorner(last.marginsAdded(margins), prevRect.marginsAdded(margins), QRectF());
    joinLeftCorner(last.marginsAdded(margins), prevRect.marginsAdded(margins), QRectF());

    for (int i = rects.count() - 2; i > 0; --i) {
        joinLeftCorner(rects.at(i) + margins, rects.at(i - 1).marginsAdded(margins), rects.at(i + 1).marginsAdded(margins));
    }

    joinLeftCorner(first, QRectF(), rects.at(1).marginsAdded(margins));

    path.closeSubpath();

    return path;
}

class TagTextFormat : public QTextCharFormat
{
public:
    TagTextFormat(int objectType, const QList<QColor> &colors, const QColor &borderColor);

    QList<QColor> colors() const;
    QColor borderColor() const;
    qreal diameter() const;
};

TagTextFormat::TagTextFormat(int objectType, const QList<QColor> &colors, const QColor &borderColor)
{
    setObjectType(objectType);
    setProperty(QTextFormat::UserProperty + 1, QVariant::fromValue(colors));
    setProperty(QTextFormat::UserProperty + 2, borderColor);
}

QList<QColor> TagTextFormat::colors() const
{
    return qvariant_cast<QList<QColor>>(property(QTextFormat::UserProperty + 1));
}

QColor TagTextFormat::borderColor() const
{
    return colorProperty(QTextFormat::UserProperty + 2);
}

qreal TagTextFormat::diameter() const
{
    return 12;
}

class FileTagObjectInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    explicit FileTagObjectInterface()
        : QObject()
    {

    }

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format) override;

    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format) override;
};

QSizeF FileTagObjectInterface::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    Q_UNUSED(doc)

    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
    const QList<QColor> &colors = f.colors();
    const double diameter = f.diameter();

    if (colors.size() == 1)
        return QSizeF(diameter, diameter);

    return QSizeF(diameter + (colors.size() - 1) * diameter / 2.0, diameter);
}

void FileTagObjectInterface::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    Q_UNUSED(doc)

    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
    const QList<QColor> &colors = f.colors();
    const QColor borderColor = f.borderColor();
    qreal diameter = f.diameter();
    const qreal padding = diameter / 10.0;
    QRectF bounding_rect = rect.marginsRemoved(QMarginsF(padding, padding, padding, padding));

    diameter -= padding * 2;

    DFMStyledItemDelegate::paintCircleList(painter, bounding_rect, diameter, colors, borderColor);
}

class ExpandedItem : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ExpandedItem(DIconItemDelegate *d, QWidget *parent = nullptr)
        : QWidget(parent)
        , delegate(d)
    {

    }

    bool event(QEvent *ee) override
    {
        if (ee->type() == QEvent::DeferredDelete) {
            if (!canDeferredDelete) {
                ee->accept();

                return true;
            }
        }

        return QWidget::event(ee);
    }

    qreal opacity() const
    {
        return m_opactity;
    }

    void setOpacity(qreal opacity)
    {
        if (qFuzzyCompare(opacity, m_opactity))
            return;

        m_opactity = opacity;

        update();
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter pa(this);

        pa.setOpacity(m_opactity);
        pa.setPen(option.palette.color(QPalette::BrightText));
        pa.setFont(option.font);

        if (!iconPixmap.isNull()) {
            pa.drawPixmap(iconGeometry().topLeft().toPoint(), iconPixmap);
        }

        if (option.text.isEmpty())
            return;

        const QMargins &margins = contentsMargins();

        QRect label_rect(TEXT_PADDING + margins.left(),
                         margins.top() + iconHeight + TEXT_PADDING + ICON_MODE_ICON_SPACING,
                         width() - TEXT_PADDING * 2 - margins.left() - margins.right(),
                         INT_MAX);

        QString str = index.data(DFileSystemModel::FileDisplayNameRole).toString();
        const QList<QRectF> &lines = delegate->drawText(index, &pa, str, label_rect, ICON_MODE_RECT_RADIUS,
                                                        option.palette.brush(QPalette::Normal, QPalette::Highlight),
                                                        QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                        option.textElideMode, Qt::AlignCenter);

        textBounding = boundingRect(lines).toRect();
    }

    QSize sizeHint() const override
    {
        return QSize(width(), FLOOR(textGeometry().bottom() + contentsMargins().bottom()));
    }

    int heightForWidth(int width) const override
    {
        if (width != this->width()) {
            textBounding = QRect();
        }

        return FLOOR(textGeometry(width).bottom() + contentsMargins().bottom());
    }

    void setIconPixmap(const QPixmap &pixmap, int height)
    {
        iconPixmap = pixmap;
        iconHeight = height;
        update();
    }

    QRectF iconGeometry() const
    {
        const QRect &content_rect = contentsRect();

        if (iconPixmap.isNull()) {
            QRectF rect(content_rect);

            rect.setHeight(iconHeight);

            return rect;
        }

        QRectF icon_rect(QPointF((content_rect.width() - iconPixmap.width() / iconPixmap.devicePixelRatio()) / 2.0,
                                 (iconHeight - iconPixmap.height() / iconPixmap.devicePixelRatio()) / 2.0 + content_rect.top()),
                         iconPixmap.size() / iconPixmap.devicePixelRatio());

        return icon_rect;
    }

    QRectF textGeometry(int width = -1) const
    {
        if (textBounding.isEmpty() && !option.text.isEmpty()) {
            const QMargins &margins = contentsMargins();

            if (width < 0)
                width = this->width();

            width -= (margins.left() + margins.right());

            QRect label_rect(TEXT_PADDING + margins.left(),
                             iconHeight + TEXT_PADDING + ICON_MODE_ICON_SPACING + margins.top(),
                             width - TEXT_PADDING * 2,
                             INT_MAX);

            QString str = index.data(DFileSystemModel::FileDisplayNameRole).toString();
            const QList<QRectF> &lines = delegate->drawText(index, nullptr, str, label_rect, ICON_MODE_RECT_RADIUS, Qt::NoBrush,
                                                            QTextOption::WrapAtWordBoundaryOrAnywhere, option.textElideMode, Qt::AlignCenter);

            textBounding = boundingRect(lines);
        }

        return textBounding;;
    }

    QPixmap iconPixmap;
    int iconHeight = 0;
    mutable QRectF textBounding;
    QModelIndex index;
    QStyleOptionViewItem option;
    qreal m_opactity = 1;
    bool canDeferredDelete = true;
    DIconItemDelegate *delegate;
};

class DIconItemDelegatePrivate : public DFMStyledItemDelegatePrivate
{
public:
    explicit DIconItemDelegatePrivate(DIconItemDelegate *qq)
        : DFMStyledItemDelegatePrivate(qq)
    {}

    QSize textSize(const QString &text, const QFontMetrics &metrics, int lineHeight = -1) const;
    QPixmap getFileIconPixmap(const QModelIndex &index, const QIcon &icon, const QSize &icon_size, QIcon::Mode mode, qreal devicePixelRatio) const;

    QPointer<ExpandedItem> expandedItem;

    //    mutable QHash<QString, QString> elideMap;
    //    mutable QHash<QString, QString> wordWrapMap;
    //    mutable QHash<QString, int> textHeightMap;
    mutable QModelIndex expandedIndex;
    mutable QModelIndex lastAndExpandedInde;

    QList<int> iconSizes;
    /// default icon size is 64px.
    int currentIconSizeIndex = 1;

    QColor focusTextBackgroundBorderColor;
    bool enabledTextShadow = false;
    // 最后一次绘制item时是否画了背景
    mutable bool drawTextBackgroundOnLast = true;

    QTextDocument *document = nullptr;

    static int textObjectType;
    static FileTagObjectInterface *textObjectInterface;

    Q_DECLARE_PUBLIC(DIconItemDelegate)
};

int DIconItemDelegatePrivate::textObjectType = QTextFormat::UserObject + 1;
FileTagObjectInterface *DIconItemDelegatePrivate::textObjectInterface = new FileTagObjectInterface();

QSize DIconItemDelegatePrivate::textSize(const QString &text, const QFontMetrics &metrics, int lineHeight) const
{
    QString str = text;

    if (str.endsWith('\n'))
        str.chop(1);

    int max_width = 0;
    int height = 0;

    for (const QString &line : str.split('\n')) {
        max_width = qMax(metrics.width(line), max_width);

        if (lineHeight > 0)
            height += lineHeight;
        else
            height += metrics.height();
    }

    return QSize(max_width, height);
}
QPixmap DIconItemDelegatePrivate::getFileIconPixmap(const QModelIndex &index, const QIcon &icon, const QSize &icon_size, QIcon::Mode mode, qreal devicePixelRatio) const
{
    Q_Q(const DIconItemDelegate);

    QPixmap pixmap = q->getIconPixmap(icon, icon_size, devicePixelRatio, mode);
    QPainter painter(&pixmap);

    /// draw file additional icon

    QList<QRectF> cornerGeometryList = q->getCornerGeometryList(QRect(QPoint(0, 0), icon_size), icon_size / 3);
    const QList<QIcon> &cornerIconList = q->parent()->additionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        if (cornerIconList.at(i).isNull()) {
            continue;
        }
        cornerIconList.at(i).paint(&painter, cornerGeometryList.at(i).toRect());
    }

    return pixmap;
}

DIconItemDelegate::DIconItemDelegate(DFileViewHelper *parent)
    : DFMStyledItemDelegate(*new DIconItemDelegatePrivate(this), parent)
    , m_checkedIcon(QIcon::fromTheme("emblem-checked"))
{
    QMutexLocker lk(&m_mutex);
    Q_D(DIconItemDelegate);

    d->expandedItem = new ExpandedItem(this, parent->parent()->viewport());
    d->expandedItem->setAttribute(Qt::WA_TransparentForMouseEvents);
    d->expandedItem->canDeferredDelete = false;
    d->expandedItem->setContentsMargins(0, 0, 0, 0);
    /// prevent flash when first call show()
    d->expandedItem->setFixedWidth(0);

    d->iconSizes << 48 << 64 << 96 << 128 << 256;

    connect(parent, &DFileViewHelper::triggerEdit, this, &DIconItemDelegate::onTriggerEdit);

    parent->parent()->setIconSize(iconSizeByIconSizeLevel());
}

DIconItemDelegate::~DIconItemDelegate()
{
    QMutexLocker lk(&m_mutex);
    Q_D(DIconItemDelegate);

    if (d->expandedItem) {
        d->expandedItem->setParent(nullptr);
        d->expandedItem->canDeferredDelete = true;
        d->expandedItem->deleteLater();
    }
}

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

void DIconItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QMutexLocker lk(&(const_cast<DIconItemDelegate *>(this)->m_mutex));
    Q_D(const DIconItemDelegate);

    bool isCanvas = parent()->property("isCanvasViewHelper").toBool();
    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = (static_cast<QPaintDevice *>(parent()->parent()->viewport()) != painter->device());
    bool isEnabled = option.state & QStyle::State_Enabled;
    bool hasFocus = option.state & QStyle::State_HasFocus;

    if (index == d->expandedIndex && !parent()->isSelected(index))
        const_cast<DIconItemDelegate *>(this)->hideNotEditingIndexWidget();

    painter->setOpacity(parent()->isTransparent(index) ? 0.3 : 1.0);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont old_font = opt.font;

    if (old_font != opt.font) {
        if (d->expandedItem)
            d->expandedItem->setFont(opt.font);

        QWidget *editing_widget = editingIndexWidget();

        if (editing_widget)
            editing_widget->setFont(opt.font);

        const_cast<DIconItemDelegate *>(this)->updateItemSizeHint();
        //        parent()->parent()->updateEditorGeometries();
    }

    old_font = opt.font;

    bool isSelected = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    bool isDropTarget = parent()->isDropTarget(index);

    DPalette pl(DApplicationHelper::instance()->palette(option.widget));
    QColor c = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    QColor base_color = c;
    if (option.widget) {
        DPalette pa = DApplicationHelper::instance()->palette(option.widget);
        base_color = option.widget->palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);
        if (ct == DGuiApplicationHelper::DarkType) {
            base_color = DGuiApplicationHelper::adjustColor(base_color, 0, 0, +5, 0, 0, 0, 0);
        }
    }

    if ((isDropTarget && !isSelected) || option.state & QStyle::StateFlag::State_Selected) {
        if (isCanvas) {
            c = pl.color(DPalette::ColorGroup::Active, QPalette::ColorRole::Highlight);
        } else {
            c.setAlpha(c.alpha() + 30);
        }
    } else if (option.state & QStyle::StateFlag::State_MouseOver) {
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);
        if (ct == DGuiApplicationHelper::DarkType) {
            base_color = DGuiApplicationHelper::adjustColor(base_color, 0, 0, +5, 0, 0, 0, 0);
            c = base_color;
        }
        else
            c = c.lighter();
    } else if (!isCanvas) {
        c = base_color;
    }

    QRectF rect = opt.rect;
    int backgroundMargin = isCanvas ? 0 : COLUMU_PADDING;
    if (!isCanvas)
        rect.adjust(backgroundMargin, backgroundMargin, -backgroundMargin, -backgroundMargin); // 为了让对勾右上角， 缩小框框

    QPainterPath path;
    rect.moveTopLeft(QPointF(0.5, 0.5) + rect.topLeft());
    path.addRoundedRect(rect, ICON_MODE_BACK_RADIUS, ICON_MODE_BACK_RADIUS);

    if (!isCanvas && !isDragMode) { // 桌面和拖拽的图标不画背景
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, c);
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    if (isDropTarget && !isSelected) {
        painter->setPen(c);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    /// init icon geomerty
    QRectF icon_rect = opt.rect;

    icon_rect.setSize(parent()->parent()->iconSize());
    double iconTopOffset = isCanvas ? 0 : (opt.rect.height() - icon_rect.height()) / 3.0;
    icon_rect.moveLeft(opt.rect.left() + (opt.rect.width() - icon_rect.width()) / 2.0);
    icon_rect.moveTop(opt.rect.top() +  iconTopOffset); // move icon down

    /// draw icon
    if (isSelected) {
        paintIcon(painter, opt.icon, icon_rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    } else if (isDropTarget) {
        QPixmap pixmap = opt.icon.pixmap(icon_rect.size().toSize());
        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), icon_rect.size().toSize()), QColor(0, 0, 0, CEIL(255 * 0.1)));
        p.end();

        painter->drawPixmap(icon_rect.toRect(), pixmap);
    } else {
        paintIcon(painter, opt.icon, icon_rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    }

    /// draw file additional icon

    const QSizeF &cornerBaseSize = icon_rect.size() / 3;
    QList<QRectF> cornerGeometryList = getCornerGeometryList(icon_rect, QSizeF(qMin(24.0, cornerBaseSize.width()), qMin(24.0, cornerBaseSize.height())));
    const QList<QIcon> &cornerIconList = parent()->additionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        if (cornerIconList.at(i).isNull()) {
            continue;
        }
        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i).toRect());
    }

    if (!isCanvas && isSelected) {
        QRect rc = option.rect;
        rc.setSize({20, 20});
        rc.moveTopRight(QPoint(option.rect.right() - 5, option.rect.top() + 5));

        DStyleOptionButton check;
        check.state = DStyle::State_Enabled | DStyle::State_On;
        check.rect = rc;

        DApplication::style()->drawPrimitive(DStyle::PE_IndicatorItemViewItemCheck, &check, painter);
    }

    if (index == d->editingIndex && !isDragMode) {
        // 正在编辑的item，不重绘text
        return;
    }

    if (index == d->expandedIndex && !isDragMode
            && d->expandedItem && d->expandedItem->index == index
            && d->expandedItem->option.rect == opt.rect) {
        // fixbug65053 屏幕数据变化后，桌面展开图标的文本位置错误
        // 被展开的item，且rect未改变时，不重绘text
        d->expandedItem->option = opt;
        return;
    }

    //icon模式下固定展示文件名
    QString str = index.data(DFileSystemModel::FileDisplayNameRole).toString();

    /// init file name geometry
    QRectF label_rect = opt.rect;
    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);
    if (isCanvas) {
        label_rect.setWidth(opt.rect.width() - 2 * TEXT_PADDING);
        label_rect.moveLeft(label_rect.left() + TEXT_PADDING);
    } else {
        label_rect.setWidth(opt.rect.width() - 2 * TEXT_PADDING - 2 * backgroundMargin - ICON_MODE_BACK_RADIUS);
        label_rect.moveLeft(label_rect.left() + TEXT_PADDING + backgroundMargin + ICON_MODE_BACK_RADIUS / 2);
    }

    //限制当前文字在圆角背景之内，否则字体将超出背景绘制框
    label_rect.setBottom(path.boundingRect().toRect().bottom());

    //文管窗口拖拽时的字体保持白色
    if ((isSelected && isCanvas) || isDragMode) {
        painter->setPen(opt.palette.color(QPalette::BrightText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Text));
    }

    /// if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexsCount() < 2;

    if (isSelected && singleSelected) {
        const_cast<DIconItemDelegate *>(this)->hideNotEditingIndexWidget();

        /// init file name text
        const QList<QRectF> &lines = drawText(index, nullptr, str, label_rect.adjusted(0, 0, 0, 99999), 0, QBrush(Qt::NoBrush));
        qreal height = boundingRect(lines).height();

        // we don't expend item in dde-fm but expand item on desktop && manager
        bool shouldExpend = true;

        if (shouldExpend && height > label_rect.height()) {
            /// use widget(FileIconItem) show file icon and file name label.
            d->expandedIndex = index;

            setEditorData(d->expandedItem, index);
            parent()->setIndexWidget(index, d->expandedItem);

            // 重设item状态
            d->expandedItem->index = index;
            d->expandedItem->option = opt;
            d->expandedItem->textBounding = QRectF();
            d->expandedItem->setFixedWidth(0);
            d->expandedItem->setContentsMargins(backgroundMargin, CEIL(iconTopOffset), backgroundMargin, 0);

            if (parent()->indexOfRow(index) == parent()->rowCount() - 1) {
                d->lastAndExpandedInde = index;
            }

            parent()->updateGeometries();

            return;
        }
    } else {
        if (!singleSelected) {
            const_cast<DIconItemDelegate *>(this)->hideNotEditingIndexWidget();
        }
    }

    if (isSelected || !d->enabledTextShadow || isDragMode) {// do not draw text background color

        //图标拖拽时保持活动色
        auto tempBackground = isDragMode ? (opt.palette.brush(QPalette::Normal, QPalette::Highlight)) : (isCanvas ? opt.palette.brush(QPalette::Normal, QPalette::Highlight) : QBrush(Qt::NoBrush));
        const QList<QRectF> &lines = drawText(index, painter, str, label_rect, ICON_MODE_RECT_RADIUS, tempBackground,
                                              QTextOption::WrapAtWordBoundaryOrAnywhere, opt.textElideMode, Qt::AlignCenter);

        const QColor &border_color = focusTextBackgroundBorderColor();

        if (hasFocus && !singleSelected && border_color.isValid()) {
            QPainterPath line_path = boundingPath(lines, ICON_MODE_RECT_RADIUS, 1);

            painter->setPen(QPen(border_color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(line_path);
        }
    } else {
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
    }

    painter->setOpacity(1);
}

bool DIconItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(DFileSystemModel::Roles::FileIconModelToolTipRole).toString();

        if (tooltip.isEmpty()) { // 当从一个需要显示tooltip的icon上移动光标到不需要显示的icon上时立即隐藏当前tooltip
            QWidgetList qwl = QApplication::topLevelWidgets();
            for (QWidget *qw : qwl) {
                if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
                    qw->close();
                }
            }
        } else {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
            QToolTip::showText(event->globalPos(), strtooltip, view);
        }

        return true;
    }

    return DFMStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize DIconItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    const QSize &size = d->itemSizeHint;

    if (index.isValid() && index == d->lastAndExpandedInde) {
        d->expandedItem->iconHeight = parent()->parent()->iconSize().height();

        return QSize(size.width(), d->expandedItem->heightForWidth(size.width()));
    }

    return size;
}

QWidget *DIconItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    d->editingIndex = index;

    FileIconItem *item = new FileIconItem(parent);

    //此处更改逻辑不再保持焦点离开后依然保持的item编辑态，将会提交相关的编辑框(与桌面保持一致)
    connect(item, &FileIconItem::inputFocusOut,
            this, &DIconItemDelegate::commitDataAndCloseActiveEditor,
            Qt::UniqueConnection);

    connect(item, &FileIconItem::destroyed, this, [this, d] {
        Q_UNUSED(this)
        QWidget *editor = this->parent()->indexWidget(d->editingIndex);
        if (!editor || editor == sender()) {
            d->editingIndex = QModelIndex();
        } else {
            qInfo() << d->editingIndex << "new edit create so not set d->editingIndex!";
        }
    });

    //编辑框的字符变更处理
    connect(item->edit, &QTextEdit::textChanged, this, [=](){

        //阻塞信号等待当前函数
        const QSignalBlocker blocker(sender());

        if (!item->edit || item->edit->isReadOnly())
            return;

        //获取之前的文件名称
        QString srcText = item->edit->toPlainText();

        //清空了当前所有文本
        if (srcText.isEmpty()) {
            //根据文本调整edit高度
            item->resizeFromEditTextChanged();
            return;
        }

        //得到处理之后的文件名称
        QString dstText = DFMGlobal::preprocessingFileName(srcText);

        //如果存在非法字符且更改了当前的文本文件
        if (srcText != dstText) {

            // 修改文件的命名规则 弹出提示框(气泡提示)
            if ( !this->parent() || !this->parent()->parent()) {
                qInfo() << "parent is nullptr";
                return;
            }

            auto view = this->parent()->parent();
            auto showPoint = view->mapToGlobal( QPoint( item->pos().x() + item->width() / 2,
                                                        item->pos().y() + item->height() - ICON_MODE_RECT_RADIUS ));
            //背板主题一致
            auto color = view->palette().background().color();

            DFMGlobal::showAlertMessage(showPoint,
                                        color,
                                        QObject::tr("%1 are not allowed").arg("|[/\\*:\"']?<>"));
            //之前的光标Pos
            int srcCursorPos = item->edit->textCursor().position();
            QSignalBlocker blocker(item->edit);
            item->edit->setPlainText(dstText);
            int endPos = srcCursorPos + (dstText.length() - srcText.length());
            //此处调整光标位置
            QTextCursor cursor = item->edit->textCursor();
            cursor.setPosition(endPos);
            item->edit->setTextCursor(cursor);
            item->edit->setAlignment(Qt::AlignHCenter);
        }

        //编辑字符的长度控制
        int editTextMaxLen = item->maxCharSize();
        int editTextCurrLen = dstText.toLocal8Bit().size();
        int editTextRangeOutLen = editTextCurrLen - editTextMaxLen;
        if (editTextRangeOutLen > 0 && editTextMaxLen != INT_MAX) {
            // fix bug 69627
            QVector<uint> list = dstText.toUcs4();
            int cursor_pos = item->edit->textCursor().position();
            while (dstText.toLocal8Bit().size() > editTextMaxLen && cursor_pos > 0) {
                list.removeAt(--cursor_pos);
                dstText = QString::fromUcs4(list.data(), list.size());
            }
            QSignalBlocker blocker(item->edit);
            item->edit->setPlainText(dstText);
            QTextCursor cursor = item->edit->textCursor();
            cursor.setPosition(cursor_pos);
            item->edit->setTextCursor(cursor);
            item->edit->setAlignment(Qt::AlignHCenter);
        }

        //根据文本调整edit高度
        item->resizeFromEditTextChanged();

        //添加到stack中
        if (item->editTextStackCurrentItem() != item->edit->toPlainText()) {
            item->pushItemToEditTextStack(item->edit->toPlainText());
        }
    }, Qt::UniqueConnection);

    //设置字体居中
    //注: 此处经过查阅发现FileItem中editUndo中没有相关设置
    //触发撤销将不会产生字体对其,这里稍微提及,方便以后更改
    item->edit->setAlignment(Qt::AlignHCenter);
    item->edit->document()->setTextWidth(d->itemSizeHint.width());
    item->setOpacity(this->parent()->isTransparent(index) ? 0.3 : 1);

    return item;
}

void DIconItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    const QSize &icon_size = parent()->parent()->iconSize();

    editor->move(option.rect.topLeft());
    editor->setMinimumHeight(option.rect.height());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (editor == d->expandedItem) {
        //重置textBounding，使其在adjustSize重新计算，否则在调整图标大小时使用旧的textBounding计算导致显示不全
        d->expandedItem->textBounding = QRect();

        editor->setFixedWidth(option.rect.width());
        d->expandedItem->iconHeight = icon_size.height();
        editor->adjustSize();
        return;
    }

    editor->setFixedWidth(option.rect.width());

    FileIconItem *item = qobject_cast<FileIconItem *>(editor);

    if (!item)
        return;

    QLabel *icon = item->icon;

    if (icon_size.height() != icon->size().height()) {
        bool isCanvas = parent()->property("isCanvasViewHelper").toBool();
        int topoffset =  isCanvas ? 0 : (opt.rect.height() - icon_size.height()) / 3;//update edit pos
        icon->setFixedHeight(icon_size.height() + topoffset);
    }
}

//item->edit->setPlainText会触发textChanged连接槽进行相关的字符处理
void DIconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyleOptionViewItem opt;

    initStyleOption(&opt, index);

    const QSize &icon_size = parent()->parent()->iconSize();

    if (ExpandedItem *item = qobject_cast<ExpandedItem *>(editor)) {
        item->iconHeight = icon_size.height();
        item->setOpacity(parent()->isTransparent(index) ? 0.3 : 1);

        return;
    }

    FileIconItem *item = qobject_cast<FileIconItem *>(editor);

    if (!item || !item->edit)
        return;

    //获取当前是否显示文件后缀
    bool donot_show_suffix{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() };

    if (item->edit->isReadOnly()) {
        item->edit->setPlainText(index.data(DFileSystemModel::FileDisplayNameRole).toString());
    } else {
        if (donot_show_suffix) {
            const QString &suffix = index.data(DFileSystemModel::FileSuffixOfRenameRole).toString();

            editor->setProperty("_d_whether_show_suffix", suffix);

            item->setMaxCharSize(MAX_FILE_NAME_CHAR_COUNT - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1));
            item->edit->setPlainText(index.data(DFileSystemModel::FileBaseNameOfRenameRole).toString());
        } else {
            item->setMaxCharSize(MAX_FILE_NAME_CHAR_COUNT);
            item->edit->setPlainText(index.data(DFileSystemModel::FileNameOfRenameRole).toString());
        }
    }

    //　源文件名称不存在更改,此处会受到textChanged的更改出现光标重新计算(更改此处请慎重)
    QString srcText;
    if (donot_show_suffix) {
        srcText = index.data(DFileSystemModel::FileBaseNameOfRenameRole).toString();
    } else {
        srcText = index.data(DFileSystemModel::FileNameOfRenameRole).toString();
    }

    int baseNameLength = index.data(DFileSystemModel::FileBaseNameOfRenameRole).toString().length();

    QString dstText = DFMGlobal::preprocessingFileName(srcText);
    if (srcText == dstText) {
        //初始化选中任意文件基本名称
        QTextCursor cursor = item->edit->textCursor();
        cursor.setPosition(0);
        cursor.setPosition(baseNameLength, QTextCursor::KeepAnchor);
        item->edit->setTextCursor(cursor);
    } else {
        // 修改文件的命名规则 弹出提示框(气泡提示)
        if (!this->parent() || !this->parent()->parent()) {
            return;
        }

        auto view = this->parent()->parent();
        auto showPoint = view->mapToGlobal(QPoint(item->pos().x() + item->width()/2,
                                                  item->pos().y() + item->height() - ICON_MODE_RECT_RADIUS ));
        //背板主题一致
        auto color = view->palette().background().color();

        DFMGlobal::showAlertMessage(showPoint,
                                    color,
                                    QObject::tr("%1 are not allowed").arg("|[/\\*:\"']?<>"));

        item->edit->setPlainText(dstText);

        int endPos = baseNameLength + (dstText.length() - srcText.length());

        QTextCursor cursor = item->edit->textCursor();
        cursor.setPosition(0);
        cursor.setPosition(endPos,QTextCursor::KeepAnchor);
        item->edit->setTextCursor(cursor);
    }

    item->edit->setAlignment(Qt::AlignHCenter);
}

QList<QRect> DIconItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    Q_UNUSED(sizeHintMode)
    Q_D(const DIconItemDelegate);

    QList<QRect> geometries;

    if (index == d->expandedIndex) {
        QRect geometry = d->expandedItem->iconGeometry().toRect();

        geometry.moveTopLeft(geometry.topLeft() + d->expandedItem->pos());

        geometries << geometry;

        geometry = d->expandedItem->textGeometry().toRect();
        geometry.moveTopLeft(geometry.topLeft() + d->expandedItem->pos());
        geometry.setTop(geometries.first().bottom());

        geometries << geometry;

        return geometries;
    }

    /// init icon geomerty

    QRect icon_rect = option.rect;

    icon_rect.setSize(parent()->parent()->iconSize());
    icon_rect.moveCenter(option.rect.center());
    icon_rect.moveTop(option.rect.top());

    geometries << icon_rect;

    QString str = index.data(Qt::DisplayRole).toString();

    if (str.isEmpty()) {
        return geometries;
    }

    /// init file name geometry

    QRect label_rect = option.rect;
    bool isCanvas = parent()->property("isCanvasViewHelper").toBool();
    int backgroundMargin = isCanvas ? 0 : COLUMU_PADDING;
    if (isCanvas) {
        label_rect.setWidth(label_rect.width() - 2 * TEXT_PADDING);
        label_rect.moveLeft(label_rect.left() + TEXT_PADDING);
    } else {
        label_rect.setWidth(label_rect.width() - 2 * TEXT_PADDING - 2 * backgroundMargin - ICON_MODE_BACK_RADIUS);
        label_rect.moveLeft(label_rect.left() + TEXT_PADDING + backgroundMargin + ICON_MODE_BACK_RADIUS / 2);
    }
    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);

    QStyleOptionViewItem opt = option;
    //    initStyleOption(&opt, index);

    bool isSelected = parent()->isSelected(index) && opt.showDecorationSelected;
    /// if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexsCount() < 2;

    QTextLayout text_layout;

    text_layout.setFont(option.font);
    text_layout.setText(str);

    bool elide = (!isSelected || !singleSelected);

    //此处慎重更改 auto lines会同步document属性,更改后导致行数的计算错误影响绘制
    auto lines = drawText(index, nullptr, str, QRect(label_rect.topLeft(), QSize(label_rect.width(), INT_MAX)),
                          ICON_MODE_RECT_RADIUS, isSelected ? opt.backgroundBrush : QBrush(Qt::NoBrush),
                          QTextOption::WrapAtWordBoundaryOrAnywhere, elide ? opt.textElideMode : Qt::ElideNone,
                          Qt::AlignCenter);

    label_rect = boundingRect(lines).toRect();
    label_rect.setTop(icon_rect.bottom());

    geometries << label_rect;

    // background rect
    QRect background_rect = option.rect;
    if (!isCanvas) {
        // 为了让对勾右上角， 缩小框框
        background_rect.adjust(backgroundMargin, backgroundMargin, -backgroundMargin, -backgroundMargin);
        geometries << background_rect;
    }

    return geometries;
}

QModelIndexList DIconItemDelegate::hasWidgetIndexs() const
{
    const QModelIndex &index = expandedIndex();

    if (!index.isValid())
        return DFMStyledItemDelegate::hasWidgetIndexs();

    return DFMStyledItemDelegate::hasWidgetIndexs() << index;
}

void DIconItemDelegate::hideNotEditingIndexWidget()
{
    Q_D(DIconItemDelegate);

    if (d->expandedIndex.isValid()) {
        parent()->setIndexWidget(d->expandedIndex, nullptr);
        d->expandedItem->hide();
        d->expandedIndex = QModelIndex();
        d->lastAndExpandedInde = QModelIndex();
    }
}

QModelIndex DIconItemDelegate::expandedIndex() const
{
    Q_D(const DIconItemDelegate);

    return d->expandedIndex;
}

QWidget *DIconItemDelegate::expandedIndexWidget() const
{
    Q_D(const DIconItemDelegate);

    return d->expandedItem;
}

int DIconItemDelegate::iconSizeLevel() const
{
    Q_D(const DIconItemDelegate);

    return d->currentIconSizeIndex;
}

int DIconItemDelegate::minimumIconSizeLevel() const
{
    return 0;
}

int DIconItemDelegate::maximumIconSizeLevel() const
{
    Q_D(const DIconItemDelegate);

    return d->iconSizes.count() - 1;
}

/*!
 * \brief Return current icon level if icon can increase; otherwise return -1.
 * \return
 */
int DIconItemDelegate::increaseIcon()
{
    Q_D(const DIconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex + 1);
}

/*!
 * \brief Return current icon level if icon can decrease; otherwise return -1.
 * \return
 */
int DIconItemDelegate::decreaseIcon()
{
    Q_D(const DIconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex - 1);
}

/*!
 * \brief Return current icon level if level is vaild; otherwise return -1.
 * \param level
 * \return
 */
int DIconItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_D(DIconItemDelegate);

    if (level == d->currentIconSizeIndex)
        return level;

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = level;

        parent()->parent()->setIconSize(iconSizeByIconSizeLevel());

        return d->currentIconSizeIndex;
    }

    return -1;
}

void DIconItemDelegate::updateItemSizeHint()
{
    Q_D(DIconItemDelegate);

    //    d->elideMap.clear();
    //    d->wordWrapMap.clear();
    //    d->textHeightMap.clear();
    d->textLineHeight = parent()->parent()->fontMetrics().lineSpacing();

    int width = parent()->parent()->iconSize().width() + 30;
    int height = parent()->parent()->iconSize().height() + 2 * COLUMU_PADDING
            + 2 * TEXT_PADDING  + ICON_MODE_ICON_SPACING + 3 * d->textLineHeight;
    int size = qMax(width, height);
    d->itemSizeHint = QSize(size, size);
    //d->itemSizeHint = QSize(width, parent()->parent()->iconSize().height() + 2 * TEXT_PADDING  + ICON_MODE_ICON_SPACING + 3 * d->textLineHeight);
}

QColor DIconItemDelegate::focusTextBackgroundBorderColor() const
{
    Q_D(const DIconItemDelegate);

    return d->focusTextBackgroundBorderColor;
}

bool DIconItemDelegate::enabledTextShadow() const
{
    Q_D(const DIconItemDelegate);

    return d->enabledTextShadow;
}

void DIconItemDelegate::setFocusTextBackgroundBorderColor(QColor focusTextBackgroundBorderColor)
{
    Q_D(DIconItemDelegate);

    d->focusTextBackgroundBorderColor = focusTextBackgroundBorderColor;

    //    if (d->expandedItem)
    //        d->expandedItem->setBorderColor(focusTextBackgroundBorderColor);
}

void DIconItemDelegate::setEnabledTextShadow(bool enabledTextShadow)
{
    Q_D(DIconItemDelegate);

    d->enabledTextShadow = enabledTextShadow;
}

void DIconItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    Q_D(const DIconItemDelegate);

    const QVariantHash &ep = index.data(DFileSystemModel::ExtraProperties).toHash();
    const QList<QColor> &colors = qvariant_cast<QList<QColor>>(ep.value("colored"));

    if (!colors.isEmpty()) {
        if (!layout->engine()->block.docHandle()) {
            if (!d->document)
                const_cast<DIconItemDelegatePrivate *>(d)->document = new QTextDocument(const_cast<DIconItemDelegate *>(this));

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

bool DIconItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            e->accept();

            return true;
        }
    } /*else if (event->type() == QEvent::FocusOut) {
        onEditWidgetFocusOut();

        return true;
    }*/

    return QStyledItemDelegate::eventFilter(object, event);
}

QList<QRectF> DIconItemDelegate::drawText(const QModelIndex &index, QPainter *painter, QTextLayout *layout,
                                          const QRectF &boundingRect, qreal radius, const QBrush &background,
                                          QTextOption::WrapMode wordWrap, Qt::TextElideMode mode,
                                          int flags, const QColor &shadowColor) const
{
    Q_D(const DIconItemDelegate);

    const_cast<DIconItemDelegatePrivate *>(d)->drawTextBackgroundOnLast = background != Qt::NoBrush;

    return DFMStyledItemDelegate::drawText(index, painter, layout, boundingRect, radius, background, wordWrap, mode, flags, shadowColor);
}

void DIconItemDelegate::onEditWidgetFocusOut()
{
    //这里判断是为了保持编辑框的状态，使编辑框一直存在。类似setEnable的状态
    if (qApp->focusWidget() && qApp->focusWidget()->window() == parent()->parent()->window()
            && qApp->focusWidget() != parent()->parent()) {

        hideAllIIndexWidget();
    }
}

void DIconItemDelegate::onTriggerEdit(const QModelIndex &index)
{
    Q_D(DIconItemDelegate);
    m_focusWindow = qApp->focusWindow();
    if (index == d->expandedIndex) {
        parent()->setIndexWidget(index, nullptr);
        d->expandedItem->hide();
        d->expandedIndex = QModelIndex();
        d->lastAndExpandedInde = QModelIndex();
        parent()->parent()->edit(index);
    }
}


QSize DIconItemDelegate::iconSizeByIconSizeLevel() const
{
    Q_D(const DIconItemDelegate);

    int size = d->iconSizes.at(d->currentIconSizeIndex);

    return QSize(size, size);
}

#include "diconitemdelegate.moc"
