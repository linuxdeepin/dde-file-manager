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

#include "iconitemdelegate.h"
#include "styleditemdelegate_p.h"
#include "base/application.h"
#include "dfm-base/dfm_base_global.h"

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
#include <QtMath>

DWIDGET_USE_NAMESPACE

namespace GlobalPrivate {

const int ICON_SPACING = 16;
const int ICON_MODE_RECT_RADIUS = 4;
const int TEXT_PADDING = 4;
const int ICON_MODE_ICON_SPACING = 5;
const int ICON_MODE_BACK_RADIUS = 18;
const int COLUMU_PADDING = 10;

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

}


//class TagTextFormat : public QTextCharFormat
//{
//public:
//    TagTextFormat(int objectType, const QList<QColor> &colors, const QColor &borderColor);

//    QList<QColor> colors() const;
//    QColor borderColor() const;
//    qreal diameter() const;
//};

//TagTextFormat::TagTextFormat(int objectType, const QList<QColor> &colors, const QColor &borderColor)
//{
//    setObjectType(objectType);
//    setProperty(QTextFormat::UserProperty + 1, QVariant::fromValue(colors));
//    setProperty(QTextFormat::UserProperty + 2, borderColor);
//}

//QList<QColor> TagTextFormat::colors() const
//{
//    return qvariant_cast<QList<QColor>>(property(QTextFormat::UserProperty + 1));
//}

//QColor TagTextFormat::borderColor() const
//{
//    return colorProperty(QTextFormat::UserProperty + 2);
//}

//qreal TagTextFormat::diameter() const
//{
//    return 12;
//}

//class FileTagObjectInterface : public QObject, public QTextObjectInterface
//{
//    Q_OBJECT
//    Q_INTERFACES(QTextObjectInterface)

//public:
//    explicit FileTagObjectInterface()
//        : QObject()
//    {

//    }

//    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
//                         const QTextFormat &format) override;

//    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
//                    int posInDocument, const QTextFormat &format) override;
//};

//QSizeF FileTagObjectInterface::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
//{
//    Q_UNUSED(posInDocument)
//    Q_UNUSED(doc)

//    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
//    const QList<QColor> &colors = f.colors();
//    const double diameter = f.diameter();

//    if (colors.size() == 1)
//        return QSizeF(diameter, diameter);

//    return QSizeF(diameter + (colors.size() - 1) * diameter / 2.0, diameter);
//}

//void FileTagObjectInterface::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
//{
//    Q_UNUSED(posInDocument)
//    Q_UNUSED(doc)

//    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
//    const QList<QColor> &colors = f.colors();
//    const QColor borderColor = f.borderColor();
//    qreal diameter = f.diameter();
//    const qreal padding = diameter / 10.0;
//    QRectF bounding_rect = rect.marginsRemoved(QMarginsF(padding, padding, padding, padding));

//    diameter -= padding * 2;

//    DFMStyledItemDelegate::paintCircleList(painter, bounding_rect, diameter, colors, borderColor);
//}

//class ExpandedItem : public QWidget
//{
//    Q_OBJECT

//    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

//public:
//    explicit ExpandedItem(DFMIconItemDelegate *d, QWidget *parent = nullptr)
//        : QWidget(parent)
//        , delegate(d)
//    {

//    }

//    bool event(QEvent *ee) override
//    {
//        if (ee->type() == QEvent::DeferredDelete) {
//            if (!canDeferredDelete) {
//                ee->accept();

//                return true;
//            }
//        }

//        return QWidget::event(ee);
//    }

//    qreal opacity() const
//    {
//        return m_opactity;
//    }

//    void setOpacity(qreal opacity)
//    {
//        if (qFuzzyCompare(opacity, m_opactity))
//            return;

//        m_opactity = opacity;

//        update();
//    }

//    void paintEvent(QPaintEvent *) override
//    {
//        QPainter pa(this);

//        pa.setOpacity(m_opactity);
//        pa.setPen(option.palette.color(QPalette::BrightText));
//        pa.setFont(option.font);

//        if (!iconPixmap.isNull()) {
//            pa.drawPixmap(iconGeometry().topLeft().toPoint(), iconPixmap);
//        }

//        if (option.text.isEmpty())
//            return;

//        const QMargins &margins = contentsMargins();

//        QRect label_rect(TEXT_PADDING + margins.left(),
//                         margins.top() + iconHeight + TEXT_PADDING + ICON_MODE_ICON_SPACING,
//                         width() - TEXT_PADDING * 2 - margins.left() - margins.right(),
//                         INT_MAX);

//        const QList<QRectF> &lines = delegate->drawText(index, &pa, option.text, label_rect, ICON_MODE_RECT_RADIUS,
//                                                        option.palette.brush(QPalette::Normal, QPalette::Highlight),
//                                                        QTextOption::WrapAtWordBoundaryOrAnywhere,
//                                                        option.textElideMode, Qt::AlignCenter);

//        textBounding = boundingRect(lines).toRect();
//    }

//    QSize sizeHint() const override
//    {
//        return QSize(width(), FLOOR(textGeometry().bottom() + contentsMargins().bottom()));
//    }

//    int heightForWidth(int width) const override
//    {
//        if (width != this->width()) {
//            textBounding = QRect();
//        }

//        return FLOOR(textGeometry(width).bottom() + contentsMargins().bottom());
//    }

//    void setIconPixmap(const QPixmap &pixmap, int height)
//    {
//        iconPixmap = pixmap;
//        iconHeight = height;
//        update();
//    }

//    QRectF iconGeometry() const
//    {
//        const QRect &content_rect = contentsRect();

//        if (iconPixmap.isNull()) {
//            QRectF rect(content_rect);

//            rect.setHeight(iconHeight);

//            return rect;
//        }

//        QRectF icon_rect(QPointF((content_rect.width() - iconPixmap.width() / iconPixmap.devicePixelRatio()) / 2.0,
//                                 (iconHeight - iconPixmap.height() / iconPixmap.devicePixelRatio()) / 2.0 + content_rect.top()),
//                         iconPixmap.size() / iconPixmap.devicePixelRatio());

//        return icon_rect;
//    }

//    QRectF textGeometry(int width = -1) const
//    {
//        if (textBounding.isEmpty() && !option.text.isEmpty()) {
//            const QMargins &margins = contentsMargins();

//            if (width < 0)
//                width = this->width();

//            width -= (margins.left() + margins.right());

//            QRect label_rect(TEXT_PADDING + margins.left(),
//                             iconHeight + TEXT_PADDING + ICON_MODE_ICON_SPACING + margins.top(),
//                             width - TEXT_PADDING * 2,
//                             INT_MAX);

//            const QList<QRectF> &lines = delegate->drawText(index, nullptr, option.text, label_rect, ICON_MODE_RECT_RADIUS, Qt::NoBrush,
//                                                            QTextOption::WrapAtWordBoundaryOrAnywhere, option.textElideMode, Qt::AlignCenter);

//            textBounding = boundingRect(lines);
//        }

//        return textBounding;;
//    }

//    QPixmap iconPixmap;
//    int iconHeight = 0;
//    mutable QRectF textBounding;
//    QModelIndex index;
//    QStyleOptionViewItem option;
//    qreal m_opactity = 1;
//    bool canDeferredDelete = true;
//    DFMIconItemDelegate *delegate;
//};

class DFMIconItemDelegatePrivate : public DFMStyledItemDelegatePrivate
{
public:
    explicit DFMIconItemDelegatePrivate(DFMIconItemDelegate *qq)
        : DFMStyledItemDelegatePrivate(qq)
    {}

    QSize textSize(const QString &text, const QFontMetrics &metrics, int lineHeight = -1) const;
    QPixmap getFileIconPixmap(const QModelIndex &index, const QIcon &icon, const QSize &icon_size, QIcon::Mode mode, qreal devicePixelRatio) const;

//    QPointer<ExpandedItem> expandedItem;

    //    mutable QHash<QString, QString> elideMap;
    //    mutable QHash<QString, QString> wordWrapMap;
    //    mutable QHash<QString, int> textHeightMap;
//    mutable QModelIndex expandedIndex;
//    mutable QModelIndex lastAndExpandedInde;

    QList<int> iconSizes;
    /// default icon size is 64px.
    int currentIconSizeIndex = 1;

    QColor focusTextBackgroundBorderColor;
    bool enabledTextShadow = false;
    // 最后一次绘制item时是否画了背景
    mutable bool drawTextBackgroundOnLast = true;

    QTextDocument *document = nullptr;

    static int textObjectType;
//    static FileTagObjectInterface *textObjectInterface;

    Q_DECLARE_PUBLIC(DFMIconItemDelegate)
};

int DFMIconItemDelegatePrivate::textObjectType = QTextFormat::UserObject + 1;
//FileTagObjectInterface *DFMIconItemDelegatePrivate::textObjectInterface = new FileTagObjectInterface();

QSize DFMIconItemDelegatePrivate::textSize(const QString &text, const QFontMetrics &metrics, int lineHeight) const
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
QPixmap DFMIconItemDelegatePrivate::getFileIconPixmap(const QModelIndex &index, const QIcon &icon, const QSize &icon_size, QIcon::Mode mode, qreal devicePixelRatio) const
{
    Q_Q(const DFMIconItemDelegate);

    QPixmap pixmap = q->getIconPixmap(icon, icon_size, devicePixelRatio, mode);
    QPainter painter(&pixmap);

    /// draw file additional icon

    QList<QRectF> cornerGeometryList = q->getCornerGeometryList(QRect(QPoint(0, 0), icon_size), icon_size / 3);
    const QList<QIcon> cornerIconList/* = q->parent()->additionalIcon(index)*/;

    for (int i = 0; i < cornerIconList.count(); ++i) {
        if (cornerIconList.at(i).isNull()) {
            continue;
        }
        cornerIconList.at(i).paint(&painter, cornerGeometryList.at(i).toRect());
    }

    return pixmap;
}

DFMIconItemDelegate::DFMIconItemDelegate(DListView *parent)
    : DFMStyledItemDelegate(*new DFMIconItemDelegatePrivate(this), parent)
    , m_checkedIcon(QIcon::fromTheme("emblem-checked"))
{
    QMutexLocker lk(&m_mutex);
    Q_D(DFMIconItemDelegate);

//    d->expandedItem = new ExpandedItem(this, parent->parent()->viewport());
//    d->expandedItem->setAttribute(Qt::WA_TransparentForMouseEvents);
//    d->expandedItem->canDeferredDelete = false;
//    d->expandedItem->setContentsMargins(0, 0, 0, 0);
//    /// prevent flash when first call show()
//    d->expandedItem->setFixedWidth(0);

    d->iconSizes << 48 << 64 << 96 << 128 << 256;

//    connect(parent, &DFileViewHelper::triggerEdit, this, &DFMIconItemDelegate::onTriggerEdit);

    parent->setIconSize(iconSizeByIconSizeLevel());
}

DFMIconItemDelegate::~DFMIconItemDelegate()
{
    QMutexLocker lk(&m_mutex);
    Q_D(DFMIconItemDelegate);
}

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

void DFMIconItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    return DFMStyledItemDelegate::paint(painter,option,index);
}

bool DFMIconItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{

    return DFMStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize DFMIconItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    Q_D(const DFMIconItemDelegate);

    const QSize &size = d->itemSizeHint;

    return size;
}

QWidget *DFMIconItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::createEditor(parent,option,index);
}

void DFMIconItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->move(option.rect.topLeft());
    editor->setMinimumHeight(option.rect.height());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
}

//item->edit->setPlainText会触发textChanged连接槽进行相关的字符处理
void DFMIconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor,index);
}

QList<QRect> DFMIconItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    return {};
}

QModelIndexList DFMIconItemDelegate::hasWidgetIndexs() const
{
    QModelIndex index;

    if (!index.isValid())
        return DFMStyledItemDelegate::hasWidgetIndexs();

    return DFMStyledItemDelegate::hasWidgetIndexs() << index;
}

void DFMIconItemDelegate::hideNotEditingIndexWidget()
{
    DFMStyledItemDelegate::hideNotEditingIndexWidget();
}

int DFMIconItemDelegate::iconSizeLevel() const
{
    Q_D(const DFMIconItemDelegate);

    return d->currentIconSizeIndex;
}

int DFMIconItemDelegate::minimumIconSizeLevel() const
{
    return 0;
}

int DFMIconItemDelegate::maximumIconSizeLevel() const
{
    Q_D(const DFMIconItemDelegate);

    return d->iconSizes.count() - 1;
}

/*!
 * \brief Return current icon level if icon can increase; otherwise return -1.
 * \return
 */
int DFMIconItemDelegate::increaseIcon()
{
    Q_D(const DFMIconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex + 1);
}

/*!
 * \brief Return current icon level if icon can decrease; otherwise return -1.
 * \return
 */
int DFMIconItemDelegate::decreaseIcon()
{
    Q_D(const DFMIconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex - 1);
}

/*!
 * \brief Return current icon level if level is vaild; otherwise return -1.
 * \param level
 * \return
 */
int DFMIconItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_D(DFMIconItemDelegate);

    if (level == d->currentIconSizeIndex)
        return level;

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = level;

        return d->currentIconSizeIndex;
    }

    return -1;
}

void DFMIconItemDelegate::updateItemSizeHint()
{
    Q_D(DFMIconItemDelegate);
}

QColor DFMIconItemDelegate::focusTextBackgroundBorderColor() const
{
    Q_D(const DFMIconItemDelegate);

    return d->focusTextBackgroundBorderColor;
}

bool DFMIconItemDelegate::enabledTextShadow() const
{
    Q_D(const DFMIconItemDelegate);

    return d->enabledTextShadow;
}

void DFMIconItemDelegate::setFocusTextBackgroundBorderColor(QColor focusTextBackgroundBorderColor)
{
    Q_D(DFMIconItemDelegate);

    d->focusTextBackgroundBorderColor = focusTextBackgroundBorderColor;
}

void DFMIconItemDelegate::setEnabledTextShadow(bool enabledTextShadow)
{
    Q_D(DFMIconItemDelegate);

    d->enabledTextShadow = enabledTextShadow;
}

void DFMIconItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    Q_UNUSED(index);
    Q_D(const DFMIconItemDelegate);

    QList<QColor> colors;

    if (!colors.isEmpty()) {
        if (!layout->engine()->block.docHandle()) {
            if (!d->document)
                const_cast<DFMIconItemDelegatePrivate *>(d)->document = new QTextDocument(const_cast<DFMIconItemDelegate *>(this));

            d->document->setPlainText(layout->text());
            d->document->setDefaultFont(layout->font());
            layout->engine()->block = d->document->firstBlock();
        }

        QTextCursor cursor(layout->engine()->docLayout()->document());

        cursor.setPosition(0);
    }
}

bool DFMIconItemDelegate::eventFilter(QObject *object, QEvent *event)
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

void DFMIconItemDelegate::onEditWidgetFocusOut()
{
    //这里判断是为了保持编辑框的状态，使编辑框一直存在。类似setEnable的状态
//    if (qApp->focusWidget() && qApp->focusWidget()->window() == parent()->parent()->window()
//            && qApp->focusWidget() != parent()->parent()) {

//        hideAllIIndexWidget();
//    }
}

void DFMIconItemDelegate::onTriggerEdit(const QModelIndex &index)
{
    Q_D(DFMIconItemDelegate);
//    m_focusWindow = qApp->focusWindow();
//    if (index == d->expandedIndex) {
//        parent()->setIndexWidget(index, nullptr);
//        d->expandedItem->hide();
//        d->expandedIndex = QModelIndex();
//        d->lastAndExpandedInde = QModelIndex();
//        parent()->parent()->edit(index);
//    }
}


QSize DFMIconItemDelegate::iconSizeByIconSizeLevel() const
{
    Q_D(const DFMIconItemDelegate);

    int size = d->iconSizes.at(d->currentIconSizeIndex);

    return QSize(size, size);
}

//#include "dfmconitemdelegate.moc"
