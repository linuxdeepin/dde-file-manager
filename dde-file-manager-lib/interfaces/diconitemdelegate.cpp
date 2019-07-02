/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <QLabel>
#include <QPainter>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <private/qtextengine_p.h>

DFM_USE_NAMESPACE

#define ICON_SPACING 16
#define ICON_MODE_RECT_RADIUS TEXT_PADDING

QString trimmedEnd(QString str)
{
    while (!str.isEmpty()) {
        switch (str.at(str.count() - 1).toLatin1()) {
        case '\t':
        case '\n':
        case '\r':
        case '\v':
        case '\f':
        case ' ':
            str.chop(1);
            break;
        default:
            return str;
        }
    }

    return str;
}

QRectF boundingRect(const QList<QRectF> rects)
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

    auto joinRightCorner = [&] (const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect) {
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

    auto joinLeftCorner = [&] (const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect) {
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

    auto preproccess = [&] (QRectF &rect, const QRectF &prev) {
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

    const TagTextFormat &f = static_cast<const TagTextFormat&>(format);
    const QList<QColor> &colors = f.colors();
    const int diameter = f.diameter();

    if (colors.size() == 1)
        return QSizeF(diameter, diameter);

    return QSizeF(diameter + (colors.size() - 1) * diameter / 2.0, diameter);
}

void FileTagObjectInterface::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    Q_UNUSED(doc)

    const TagTextFormat &f = static_cast<const TagTextFormat&>(format);
    const QList<QColor> &colors = f.colors();
    const QColor borderColor = f.borderColor();
    qreal diameter = f.diameter();
    const qreal padding = diameter / 10.0;
    QRectF boundingRect = rect.marginsRemoved(QMarginsF(padding, padding, padding, padding));

    diameter -= padding * 2;

    DStyledItemDelegate::paintCircleList(painter, boundingRect, diameter, colors, borderColor);
}

class ExpandedItem : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ExpandedItem(DIconItemDelegate *d, QWidget *parent = 0)
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
        QRect label_rect(TEXT_PADDING + margins.left(), margins.top() + iconHeight + TEXT_PADDING + ICON_MODE_ICON_SPACING,
                         width() - TEXT_PADDING * 2 - margins.left() - margins.right(), INT_MAX);
        const QList<QRectF> &lines = delegate->drawText(index, &pa, option.text, label_rect, ICON_MODE_RECT_RADIUS,
                                                        option.backgroundBrush,
                                                        QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                        option.textElideMode, Qt::AlignCenter);

        textBounding = boundingRect(lines).toRect();
    }

    QSize sizeHint() const override
    {
        return QSize(width(), textGeometry().bottom() + contentsMargins().bottom());
    }

    int heightForWidth(int width) const override
    {
        if (width != this->width()) {
            textBounding = QRect();
        }

        return textGeometry(width).bottom() + contentsMargins().bottom();
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

            QRect label_rect(TEXT_PADDING + margins.left(), iconHeight + TEXT_PADDING + ICON_MODE_ICON_SPACING + margins.top(), width - TEXT_PADDING * 2, INT_MAX);
            const QList<QRectF> &lines = delegate->drawText(index, nullptr, option.text, label_rect, ICON_MODE_RECT_RADIUS, Qt::NoBrush,
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

class DIconItemDelegatePrivate : public DStyledItemDelegatePrivate
{
public:
    DIconItemDelegatePrivate(DIconItemDelegate *qq)
        : DStyledItemDelegatePrivate(qq)
    {}

    QSize textSize(const QString &text, const QFontMetrics &metrics, int lineHeight = -1) const;
    void drawText(QPainter *painter, const QRect &r, const QString &text,
                  int lineHeight = -1, QRect *br = Q_NULLPTR) const;
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

void DIconItemDelegatePrivate::drawText(QPainter *painter, const QRect &r, const QString &text, int lineHeight, QRect *br) const
{
    if (lineHeight <= 0)
        lineHeight = textLineHeight;

    QString str = text;

    if (str.endsWith('\n'))
        str.chop(1);

    QRect textRect = QRect(0, r.top(), 0, 0);

    for (const QString &line : str.split('\n')) {
        QRect br;

        painter->drawText(r.left(), textRect.bottom(), r.width(), lineHeight, Qt::AlignCenter, line, &br);
        textRect.setWidth(qMax(textRect.width(), br.width()));
        textRect.setBottom(textRect.bottom() + lineHeight);
    }

    if (br)
        *br = textRect;
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
        cornerIconList.at(i).paint(&painter, cornerGeometryList.at(i).toRect());
    }

    return pixmap;
}

DIconItemDelegate::DIconItemDelegate(DFileViewHelper *parent) :
    DStyledItemDelegate(*new DIconItemDelegatePrivate(this), parent)
{
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
    Q_D(DIconItemDelegate);

    if (d->expandedItem) {
        d->expandedItem->setParent(0);
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
    Q_D(const DIconItemDelegate);

    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = ((QPaintDevice*)parent()->parent()->viewport() != painter->device());
    bool isEnabled = option.state & QStyle::State_Enabled;
    bool hasFocus = option.state & QStyle::State_HasFocus;

    if (index == d->expandedIndex && !parent()->isSelected(index))
        const_cast<DIconItemDelegate*>(this)->hideNotEditingIndexWidget();

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

        const_cast<DIconItemDelegate*>(this)->updateItemSizeHint();
//        parent()->parent()->updateEditorGeometries();
    }

    old_font = opt.font;

    bool isSelected = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    bool isDropTarget = parent()->isDropTarget(index);

    if (isDropTarget && !isSelected) {
        QRectF rect = opt.rect;
        QPainterPath path;

        rect.moveTopLeft(QPointF(0.5, 0.5) + rect.topLeft());
        path.addRoundedRect(rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, QColor(43, 167, 248, 0.50 * 255));
        painter->setPen(opt.backgroundBrush.color());
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    /// init icon geomerty

    QRectF icon_rect = opt.rect;

    icon_rect.setSize(parent()->parent()->iconSize());
    icon_rect.moveLeft(opt.rect.left() + (opt.rect.width() - icon_rect.width()) / 2.0);
    icon_rect.moveTop(opt.rect.top());

    /// draw icon

    if (isSelected) {
        paintIcon(painter, opt.icon, icon_rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    } else if (isDropTarget) {
        QPixmap pixmap = opt.icon.pixmap(icon_rect.size().toSize());
        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), icon_rect.size().toSize()), QColor(0, 0, 0, 255 * 0.1));
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
        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i).toRect());
    }

//    /// draw file name label
//    if (isSelected) {
//        QPainterPath path;

//        QRect label_background_rect = label_rect;

//        label_background_rect.setSize(d->textSize(str, painter->fontMetrics(), d->textLineHeight));
//        label_background_rect.moveLeft(label_rect.left() + (label_rect.width() - label_background_rect.width()) / 2);
//        label_background_rect += QMargins(TEXT_PADDING, TEXT_PADDING, TEXT_PADDING, TEXT_PADDING);

//        path.addRoundedRect(label_background_rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing);
//        painter->fillPath(path, opt.backgroundBrush);
//        if (hasFocus && !singleSelected) {
//            painter->setPen(QPen(focusTextBackgroundBorderColor(), 2));
//            painter->drawPath(path);
//        }
//        painter->restore();
//    } else {
//        painter->fillRect(label_rect, Qt::transparent);
//    }

    if ((index == d->expandedIndex || index == d->editingIndex) && !isDragMode) {
        return;
    }

    QString str = opt.text;

    /// init file name geometry

    QRectF label_rect = opt.rect;

    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);
    label_rect.setWidth(opt.rect.width() - 2 * TEXT_PADDING);
    label_rect.moveLeft(label_rect.left() + TEXT_PADDING);

    if (isSelected) {
        painter->setPen(opt.palette.color(QPalette::BrightText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Text));
    }

    /// if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexsCount() < 2;

    if (isSelected && singleSelected) {
        const_cast<DIconItemDelegate*>(this)->hideNotEditingIndexWidget();

        int height = 0;

        /// init file name text

//        if (d->textHeightMap.contains(str)) {
//            str = d->wordWrapMap.value(str);
//            height = d->textHeightMap.value(str);
//        } else {
            const QList<QRectF> &lines = drawText(index, 0, str, label_rect.adjusted(0, 0, 0, 99999), 0, QBrush(Qt::NoBrush));
//            wordWrap_str = trimmedEnd(wordWrap_str);

//            d->wordWrapMap[str] = wordWrap_str;
            height = boundingRect(lines).height();
//            d->textHeightMap[str] = height;
//            str = wordWrap_str;
//        }

        if (height > label_rect.height()) {
            /// use widget(FileIconItem) show file icon and file name label.

            d->expandedIndex = index;

            setEditorData(d->expandedItem, index);
            parent()->setIndexWidget(index, d->expandedItem);

            // 重设item状态
            d->expandedItem->index = index;
            d->expandedItem->option = opt;
            d->expandedItem->textBounding = QRectF();
            d->expandedItem->setFixedWidth(0);

            if (parent()->indexOfRow(index) == parent()->rowCount() - 1) {
                d->lastAndExpandedInde = index;
            }

            parent()->updateGeometries();

            return;
        }
    } else {
//        /// init file name text
//        if (d->elideMap.contains(str)) {
//            str = d->elideMap.value(str);
//        } else {
//            QString elide_str = DFMGlobal::elideText(str, label_rect.size(),
//                                                     QTextOption::WrapAtWordBoundaryOrAnywhere,
//                                                     opt.font,
//                                                     opt.textElideMode, d->textLineHeight);

//            elide_str = trimmedEnd(elide_str);

//            d->elideMap[str] = elide_str;

//            str = elide_str;
//        }

        if (!singleSelected) {
            const_cast<DIconItemDelegate*>(this)->hideNotEditingIndexWidget();
        }
    }

    if (isSelected || !d->enabledTextShadow) {
        const QList<QRectF> &lines = drawText(index, painter, str, label_rect, ICON_MODE_RECT_RADIUS,
                                              isSelected ? opt.backgroundBrush : QBrush(Qt::NoBrush),
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

    connect(item, &FileIconItem::inputFocusOut, this, &DIconItemDelegate::onEditWidgetFocusOut);
    connect(item, &FileIconItem::destroyed, this, [this, d] {
        d->editingIndex = QModelIndex();
    });

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
        if (editor->width() != option.rect.width()) {
            editor->setFixedWidth(option.rect.width());
            d->expandedItem->iconHeight = icon_size.height();
            editor->adjustSize();
        }

        return;
    }

    editor->setFixedWidth(option.rect.width());

    FileIconItem *item = qobject_cast<FileIconItem*>(editor);

    if (!item)
        return;

    QLabel *icon = item->icon;

    if (icon_size.height() != icon->size().height()) {
        icon->setFixedHeight(icon_size.height());
    }
}

void DIconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    QStyleOptionViewItem opt;

    initStyleOption(&opt, index);

    const QSize &icon_size = parent()->parent()->iconSize();

    if (ExpandedItem *item = qobject_cast<ExpandedItem*>(editor)) {
        item->iconHeight = icon_size.height();
        item->setOpacity(parent()->isTransparent(index) ? 0.3 : 1);

        return;
    }

    FileIconItem *item = qobject_cast<FileIconItem*>(editor);

    if (!item)
        return;

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



    item->edit->setAlignment(Qt::AlignHCenter);
    item->edit->document()->setTextWidth(d->itemSizeHint.width());
    item->setOpacity(parent()->isTransparent(index) ? 0.3 : 1);

    if(item->edit->isReadOnly())
        return;

    const QString &selectionWhenEditing = parent()->baseName(index);
    int endPos = selectionWhenEditing.isEmpty() ? -1 : selectionWhenEditing.length();

    if (endPos == -1 || donot_show_suffix) {
        item->edit->selectAll();
    } else {
        QTextCursor cursor = item->edit->textCursor();

        cursor.setPosition(0);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);

        item->edit->setTextCursor(cursor);
    }
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

    label_rect.setWidth(label_rect.width() - 2 * TEXT_PADDING);
    label_rect.moveLeft(label_rect.left() + TEXT_PADDING);
    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);

    QStyleOptionViewItem opt = option;
//    initStyleOption(&opt, index);

    bool isSelected = parent()->isSelected(index) && opt.showDecorationSelected;
    /// if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexsCount() < 2;

//    if (isSelected && singleSelected) {
//        int height = 0;

//        if (d->wordWrapMap.contains(str)) {
//            str = d->wordWrapMap.value(str);
//            height = d->textHeightMap.value(str);
//        } else {
//            QString wordWrap_str = DFMGlobal::wordWrapText(str, label_rect.width(),
//                                                           QTextOption::WrapAtWordBoundaryOrAnywhere,
//                                                           opt.font,
//                                                           d->textLineHeight,
//                                                           &height);

//            wordWrap_str = trimmedEnd(wordWrap_str);

//            d->wordWrapMap[str] = wordWrap_str;
//            d->textHeightMap[wordWrap_str] = height;
//            str = wordWrap_str;
//        }
//    } else {
//        if (d->elideMap.contains(str)) {
//            str = d->elideMap.value(str);
//        } else {
//            QString elide_str = DFMGlobal::elideText(str, label_rect.size(), QTextOption::WrapAtWordBoundaryOrAnywhere,
//                                                     opt.font, opt.textElideMode, d->textLineHeight);

//            d->elideMap[str] = elide_str;
//            str = elide_str;
//        }
//    }

//    /// draw icon and file name label

//    label_rect = option.fontMetrics.boundingRect(label_rect, Qt::AlignHCenter, str);

    QTextLayout text_layout;

    text_layout.setFont(option.font);
    text_layout.setText(str);

    bool elide = (!isSelected || !singleSelected);

    auto lines = drawText(index, nullptr, str, QRect(label_rect.topLeft(), QSize(label_rect.width(), INT_MAX)),
                          ICON_MODE_RECT_RADIUS, isSelected ? opt.backgroundBrush : QBrush(Qt::NoBrush),
                          QTextOption::WrapAtWordBoundaryOrAnywhere, elide ? opt.textElideMode : Qt::ElideNone,
                          Qt::AlignCenter);

    label_rect = boundingRect(lines).toRect();
    label_rect.setTop(icon_rect.bottom());

    geometries << label_rect;

    return geometries;
}

QModelIndexList DIconItemDelegate::hasWidgetIndexs() const
{
    const QModelIndex &index = expandedIndex();

    if (!index.isValid())
        return DStyledItemDelegate::hasWidgetIndexs();

    return DStyledItemDelegate::hasWidgetIndexs() << index;
}

void DIconItemDelegate::hideNotEditingIndexWidget()
{
    Q_D(DIconItemDelegate);

    if (d->expandedIndex.isValid()) {
        parent()->setIndexWidget(d->expandedIndex, 0);
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
    d->textLineHeight = parent()->parent()->fontMetrics().height();

    int width = parent()->parent()->iconSize().width() + 30;

    d->itemSizeHint = QSize(width, parent()->parent()->iconSize().height() + 2 * TEXT_PADDING  + ICON_MODE_ICON_SPACING + 3 * d->textLineHeight);
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
                const_cast<DIconItemDelegatePrivate*>(d)->document = new QTextDocument(const_cast<DIconItemDelegate*>(this));

            d->document->setPlainText(layout->text());
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
        QKeyEvent *e = static_cast<QKeyEvent*>(event);

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

    const_cast<DIconItemDelegatePrivate*>(d)->drawTextBackgroundOnLast = background != Qt::NoBrush;

    return DStyledItemDelegate::drawText(index, painter, layout, boundingRect, radius, background, wordWrap, mode, flags, shadowColor);
}

void DIconItemDelegate::onEditWidgetFocusOut()
{
    if (qApp->focusWidget() && qApp->focusWidget()->window() == parent()->parent()->window()
            && qApp->focusWidget() != parent()->parent()) {

        hideAllIIndexWidget();
    }
}

void DIconItemDelegate::onTriggerEdit(const QModelIndex &index)
{
    Q_D(DIconItemDelegate);

    if(index == d->expandedIndex) {
        parent()->setIndexWidget(index, 0);
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
