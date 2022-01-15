/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#include "private/delegatecommon.h"
#include "iconitemdelegate.h"
#include "private/iconitemdelegate_p.h"
#include "dfm-base/dfm_base_global.h"
#include "utils/itemdelegatehelper.h"
#include "fileviewitem.h"
#include "fileview.h"

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
#include <QToolTip>
#include <QtMath>

DWIDGET_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

IconItemDelegate::IconItemDelegate(FileView *parent)
    : BaseItemDelegate(*new IconItemDelegatePrivate(this), parent)
{
    Q_D(IconItemDelegate);

    d->itemIconSize = iconSizeByIconSizeLevel();
}

IconItemDelegate::~IconItemDelegate()
{
}

void IconItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    D_DC(IconItemDelegate);
    bool isEnabled = option.state & QStyle::State_Enabled;
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont oldFont = opt.font;
    oldFont = opt.font;

    // init background color and geomerty
    DPalette pl(DApplicationHelper::instance()->palette(option.widget));
    QColor backgroundColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    QColor baseColor = backgroundColor;
    if (option.widget) {
        DPalette pa = DApplicationHelper::instance()->palette(option.widget);
        baseColor = option.widget->palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
        }
    }

    if (option.state & QStyle::StateFlag::State_Selected) {
        backgroundColor.setAlpha(backgroundColor.alpha() + 30);
    } else if (option.state & QStyle::StateFlag::State_MouseOver) {
        backgroundColor = backgroundColor.lighter();
    } else {
        backgroundColor = baseColor;
    }

    QRectF backgroundRect = opt.rect;
    int backgroundMargin = kIconModeColumnPadding;
    backgroundRect.adjust(backgroundMargin, backgroundMargin, -backgroundMargin, -backgroundMargin);

    // draw background
    QPainterPath path;
    backgroundRect.moveTopLeft(QPointF(0.5, 0.5) + backgroundRect.topLeft());
    path.addRoundedRect(backgroundRect, kIconModeBackRadius, kIconModeBackRadius);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillPath(path, backgroundColor);
    painter->setRenderHint(QPainter::Antialiasing, false);

    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    if (isSelected) {
        QRect rc = option.rect;
        rc.setSize({ 30, 30 });
        rc.moveTopRight(QPoint(option.rect.right(), option.rect.top()));
        d->checkedIcon.paint(painter, rc);
    }

    // init icon geomerty
    QRectF iconRect = opt.rect;
    iconRect.setSize(d->itemIconSize);
    double iconTopOffset = (opt.rect.height() - iconRect.height()) / 3.0;
    iconRect.moveLeft(opt.rect.left() + (opt.rect.width() - iconRect.width()) / 2.0);
    iconRect.moveTop(opt.rect.top() + iconTopOffset);   // move icon down
    // draw icon
    ItemDelegateHelper::paintIcon(painter, opt.icon, iconRect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);

    // init file name geometry
    QRectF labelRect = opt.rect;
    labelRect.setTop(iconRect.bottom() + kIconModeTextPadding + kIconModeIconSpacing);
    labelRect.setWidth(opt.rect.width() - 2 * kIconModeTextPadding - 2 * backgroundMargin - kIconModeBackRadius);
    labelRect.moveLeft(labelRect.left() + kIconModeTextPadding + backgroundMargin + kIconModeBackRadius / 2);
    labelRect.setBottom(path.boundingRect().toRect().bottom());
    // draw file name
    QString displayString = ItemDelegateHelper::elideText(index.data(FileViewItem::kItemNameRole).toString(), labelRect.size(),
                                                          QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                          option.font, opt.textElideMode,
                                                          GlobalPrivate::kListEditorHeight);
    painter->drawText(labelRect, Qt::AlignHCenter | Qt::AlignVCenter, displayString);

    painter->setOpacity(1);
}

bool IconItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QList<QRect> IconItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    Q_UNUSED(sizeHintMode)

    return QList<QRect>();
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
    return kIconSizeList.count() - 1;
}

int IconItemDelegate::increaseIcon()
{
    Q_D(const IconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex + 1);
}

int IconItemDelegate::decreaseIcon()
{
    Q_D(const IconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex - 1);
}

int IconItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_D(IconItemDelegate);

    if (level == iconSizeLevel()) {
        return level;
    }

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = level;
        d->itemIconSize = iconSizeByIconSizeLevel();
        d->fileView->setIconSize(iconSizeByIconSizeLevel());
        return d->currentIconSizeIndex;
    }

    return -1;
}

QSize IconItemDelegate::iconSizeByIconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    int size = kIconSizeList.at(d->currentIconSizeIndex);

    return QSize(size, size);
}

QSize IconItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    D_DC(IconItemDelegate);

    int height = d->itemIconSize.height()
            + 2 * kIconModeColumnPadding   // 上下两个icon的间距
            + 3 * 21   // 3行文字的高度
            + 2 * kIconModeTextPadding   // 文字两边的间距
            + kIconModeIconSpacing;   // icon的间距

    return QSize(height, height);
}

QWidget *IconItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void IconItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->move(option.rect.topLeft());
    editor->setMinimumHeight(option.rect.height());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
}

void IconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

bool IconItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            e->accept();

            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

//#include "dfmconitemdelegate.moc"
