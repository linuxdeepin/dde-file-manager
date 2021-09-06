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
#include "private/styleditemdelegate_p.h"
#include "services/dfm-business-services/dfm-filemanager-service/applicationservice/application.h"
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
DFMBASE_BEGIN_NAMESPACE
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

class IconItemDelegatePrivate : public StyledItemDelegatePrivate
{
public:
    explicit IconItemDelegatePrivate(IconItemDelegate *qq)
        : StyledItemDelegatePrivate(qq)
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

    Q_DECLARE_PUBLIC(IconItemDelegate)
};

int IconItemDelegatePrivate::textObjectType = QTextFormat::UserObject + 1;
//FileTagObjectInterface *DFMIconItemDelegatePrivate::textObjectInterface = new FileTagObjectInterface();

QSize IconItemDelegatePrivate::textSize(const QString &text, const QFontMetrics &metrics, int lineHeight) const
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
QPixmap IconItemDelegatePrivate::getFileIconPixmap(const QModelIndex &index, const QIcon &icon, const QSize &icon_size, QIcon::Mode mode, qreal devicePixelRatio) const
{
    Q_Q(const IconItemDelegate);

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

IconItemDelegate::IconItemDelegate(DListView *parent)
    : StyledItemDelegate(*new IconItemDelegatePrivate(this), parent)
    , m_checkedIcon(QIcon::fromTheme("emblem-checked"))
{
    QMutexLocker lk(&m_mutex);
    Q_D(IconItemDelegate);

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

IconItemDelegate::~IconItemDelegate()
{
    QMutexLocker lk(&m_mutex);
    Q_D(IconItemDelegate);
}

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

void IconItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    return StyledItemDelegate::paint(painter,option,index);
}

bool IconItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{

    return StyledItemDelegate::helpEvent(event, view, option, index);
}

QSize IconItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    Q_D(const IconItemDelegate);

    const QSize &size = d->itemSizeHint;

    return size;
}

QWidget *IconItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::createEditor(parent,option,index);
}

void IconItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->move(option.rect.topLeft());
    editor->setMinimumHeight(option.rect.height());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
}

//item->edit->setPlainText会触发textChanged连接槽进行相关的字符处理
void IconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor,index);
}

QList<QRect> IconItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    return {};
}

QModelIndexList IconItemDelegate::hasWidgetIndexs() const
{
    QModelIndex index;

    if (!index.isValid())
        return StyledItemDelegate::hasWidgetIndexs();

    return StyledItemDelegate::hasWidgetIndexs() << index;
}

void IconItemDelegate::hideNotEditingIndexWidget()
{
    StyledItemDelegate::hideNotEditingIndexWidget();
}

int IconItemDelegate::iconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    return d->currentIconSizeIndex;
}

int IconItemDelegate::minimumIconSizeLevel() const
{
    return 0;
}

int IconItemDelegate::maximumIconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    return d->iconSizes.count() - 1;
}

/*!
 * \brief Return current icon level if icon can increase; otherwise return -1.
 * \return
 */
int IconItemDelegate::increaseIcon()
{
    Q_D(const IconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex + 1);
}

/*!
 * \brief Return current icon level if icon can decrease; otherwise return -1.
 * \return
 */
int IconItemDelegate::decreaseIcon()
{
    Q_D(const IconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex - 1);
}

/*!
 * \brief Return current icon level if level is vaild; otherwise return -1.
 * \param level
 * \return
 */
int IconItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_D(IconItemDelegate);

    if (level == d->currentIconSizeIndex)
        return level;

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = level;

        return d->currentIconSizeIndex;
    }

    return -1;
}

void IconItemDelegate::updateItemSizeHint()
{
    Q_D(IconItemDelegate);
}

QColor IconItemDelegate::focusTextBackgroundBorderColor() const
{
    Q_D(const IconItemDelegate);

    return d->focusTextBackgroundBorderColor;
}

bool IconItemDelegate::enabledTextShadow() const
{
    Q_D(const IconItemDelegate);

    return d->enabledTextShadow;
}

void IconItemDelegate::setFocusTextBackgroundBorderColor(QColor focusTextBackgroundBorderColor)
{
    Q_D(IconItemDelegate);

    d->focusTextBackgroundBorderColor = focusTextBackgroundBorderColor;
}

void IconItemDelegate::setEnabledTextShadow(bool enabledTextShadow)
{
    Q_D(IconItemDelegate);

    d->enabledTextShadow = enabledTextShadow;
}

void IconItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    Q_UNUSED(index);
    Q_D(const IconItemDelegate);

    QList<QColor> colors;

    if (!colors.isEmpty()) {
        if (!layout->engine()->block.docHandle()) {
            if (!d->document)
                const_cast<IconItemDelegatePrivate *>(d)->document = new QTextDocument(const_cast<IconItemDelegate *>(this));

            d->document->setPlainText(layout->text());
            d->document->setDefaultFont(layout->font());
            layout->engine()->block = d->document->firstBlock();
        }

        QTextCursor cursor(layout->engine()->docLayout()->document());

        cursor.setPosition(0);
    }
}

bool IconItemDelegate::eventFilter(QObject *object, QEvent *event)
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

void IconItemDelegate::onEditWidgetFocusOut()
{
    //这里判断是为了保持编辑框的状态，使编辑框一直存在。类似setEnable的状态
//    if (qApp->focusWidget() && qApp->focusWidget()->window() == parent()->parent()->window()
//            && qApp->focusWidget() != parent()->parent()) {

//        hideAllIIndexWidget();
//    }
}

void IconItemDelegate::onTriggerEdit(const QModelIndex &index)
{
    Q_D(IconItemDelegate);
//    m_focusWindow = qApp->focusWindow();
//    if (index == d->expandedIndex) {
//        parent()->setIndexWidget(index, nullptr);
//        d->expandedItem->hide();
//        d->expandedIndex = QModelIndex();
//        d->lastAndExpandedInde = QModelIndex();
//        parent()->parent()->edit(index);
//    }
}


QSize IconItemDelegate::iconSizeByIconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    int size = d->iconSizes.at(d->currentIconSizeIndex);

    return QSize(size, size);
}
DFMBASE_END_NAMESPACE

//#include "dfmconitemdelegate.moc"
