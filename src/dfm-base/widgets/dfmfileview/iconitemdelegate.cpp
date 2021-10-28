/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "iconitemdelegate.h"
#include "private/iconitemdelegate_p.h"
#include "dfm-base/dfm_base_global.h"
#include "itemdelegatehelper.h"
#include "fileviewitem.h"

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
DFMBASE_BEGIN_NAMESPACE

IconItemDelegatePrivate::IconItemDelegatePrivate(IconItemDelegate *qq)
    : QObject(qq)
    , q(qq)
{

}

IconItemDelegate::IconItemDelegate(DTK_WIDGET_NAMESPACE::DListView *parent)
    : QStyledItemDelegate(parent)
    , d(new IconItemDelegatePrivate(this))
{
    d->itemIconSize.setWidth(d->sizeList[2]);
    d->itemIconSize.setHeight(d->sizeList[2]);
}

IconItemDelegate::~IconItemDelegate()
{

}

void IconItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
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
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
            backgroundColor = baseColor;
        }
        else
            backgroundColor = backgroundColor.lighter();
    }

    QRectF backgroundRect = opt.rect;
    int backgroundMargin = ICON_MODE_COLUMU_PADDING;
    backgroundRect.adjust(backgroundMargin, backgroundMargin, -backgroundMargin, -backgroundMargin);
    // draw background
    QPainterPath path;
    backgroundRect.moveTopLeft(QPointF(0.5, 0.5) + backgroundRect.topLeft());
    path.addRoundedRect(backgroundRect, ICON_MODE_BACK_RADIUS, ICON_MODE_BACK_RADIUS);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillPath(path, backgroundColor);
    painter->setRenderHint(QPainter::Antialiasing, false);
    // init icon geomerty
    QRectF iconRect = opt.rect;
    iconRect.setSize(d->itemIconSize);
    double iconTopOffset = (opt.rect.height() - iconRect.height()) / 3.0;
    iconRect.moveLeft(opt.rect.left() + (opt.rect.width() - iconRect.width()) / 2.0);
    iconRect.moveTop(opt.rect.top() +  iconTopOffset); // move icon down
    // draw icon
    ItemDelegateHelper::paintIcon(painter, opt.icon, iconRect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);

    // init file name geometry
    QRectF labelRect = opt.rect;
    labelRect.setTop(iconRect.bottom() + ICON_MODE_TEXT_PADDING + ICON_MODE_ICON_SPACING);
    labelRect.setWidth(opt.rect.width() - 2 * ICON_MODE_TEXT_PADDING - 2 * backgroundMargin - ICON_MODE_BACK_RADIUS);
    labelRect.moveLeft(labelRect.left() + ICON_MODE_TEXT_PADDING + backgroundMargin + ICON_MODE_BACK_RADIUS / 2);
    labelRect.setBottom(path.boundingRect().toRect().bottom());
    // draw file name
    painter->drawText(labelRect, Qt::AlignHCenter | Qt::AlignVCenter, opt.text);

    painter->setOpacity(1);
}

bool IconItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize IconItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    //    QStyledItemDelegate::sizeHint(option,index);

    int height = d->itemIconSize.height() + 2 * ICON_MODE_COLUMU_PADDING/*上下两个icon的间距*/ +
            3 * 21/*3行文字的高度*/ + 2 * ICON_MODE_TEXT_PADDING/*文字两边的间距*/ +
            ICON_MODE_ICON_SPACING/*icon的间距*/;
    return QSize(height, height);
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

void IconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor,index);
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

DFMBASE_END_NAMESPACE

//#include "dfmconitemdelegate.moc"
