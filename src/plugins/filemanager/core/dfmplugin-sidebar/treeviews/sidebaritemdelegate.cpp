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
#include "sidebaritemdelegate.h"

#include "sidebaritem.h"
#include "sidebarmodel.h"
#include "sidebarview.h"
#include "events/sidebareventcaller.h"

#include "dfm-base/base/schemefactory.h"

#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <DPalette>
#include <qdrawutil.h>
#include <QLineEdit>
#include <QImage>
#include <QFontMetrics>
#include <QEvent>
#include <QMouseEvent>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static constexpr int kRadius = 8;
static constexpr int kItemMargin = 10;
static constexpr int kItemIconSize= 18;
static constexpr int kEjectIconSize= 16;

namespace GlobalPrivate {
const static char *const kRegPattern { "^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*" };
void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option)
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 0, yPoint, option.rect.width(), yPoint, option.palette);

    painter->restore();
}
}   // namespace GlobalPrivate

SideBarItemDelegate::SideBarItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void SideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);
    painter->save();

    QStyleOptionViewItem opt = option;

    QStyledItemDelegate::initStyleOption(&opt, index);
    painter->setRenderHint(QPainter::Antialiasing);

    DPalette palette(DPaletteHelper::instance()->palette(option.widget));
    auto baseColor = palette.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);

    auto widgetColor = option.widget->palette().base().color();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        widgetColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);

    QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
    SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);
    QRect itemRect = opt.rect;
    QPoint dx = QPoint(kItemMargin, 0);
    QPoint dw = QPoint(-12, 0);
    bool selected = opt.state.testFlag(QStyle::State_Selected);
    QRect r(itemRect.topLeft() + dx, itemRect.bottomRight() + dw);
    if (selected) {   //Draw selected background
        QPalette::ColorGroup colorGroup = QPalette::Normal;
        QColor bgColor = option.palette.color(colorGroup, QPalette::Highlight);
        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(r, kRadius, kRadius);
    } else if (opt.state.testFlag(QStyle::State_MouseOver)) {   //Draw mouse over background
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            baseColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);
        else
            baseColor = baseColor.lighter();

        painter->setBrush(baseColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(r, kRadius, kRadius);
        if (separatorItem) {   //Draw expand icon
            QSize expandIconSize(12, 12);
            QColor c(Qt::lightGray);
            painter->setBrush(c);
            painter->setPen(Qt::NoPen);
            QPoint tl = r.topRight() + QPoint(-26, kItemMargin);
            QPoint br = r.topRight() + QPoint(0 - kItemMargin, 27);
            QRect gRect(tl, br);
            painter->drawRoundedRect(gRect, kRadius, kRadius);
            QPixmap pixmap = QIcon::fromTheme(separatorItem->isExpanded() ? "go-up" : "go-down").pixmap(expandIconSize);
            painter->drawPixmap(gRect.topRight() + QPointF(-14, 3), pixmap);
        }
    }

    QFontMetrics metricsLabel(option.widget->font());
    //Draw item icon
    bool isEjectable = false;
    QSize iconSize(kItemIconSize, kItemIconSize);
    QSize ejectIconSize(kEjectIconSize, kEjectIconSize);
    qreal iconDx = 2 * kItemMargin;
    qreal iconDy = (itemRect.height() - iconSize.height()) / 2 + 1;
    if (item) {
        if (!separatorItem) {
            painter->drawPixmap(itemRect.topLeft() + QPointF(iconDx, iconDy), item->icon().pixmap(iconSize));
            SideBarItem *sidebarItem = dynamic_cast<SideBarItem *>(item);
            if (sidebarItem) {
                ItemInfo info = sidebarItem->itemInfo();
                if (info.isEjectable) {
                    painter->drawPixmap(itemRect.bottomRight() + QPoint(0 - ejectIconSize.width() * 2, 0 - (itemRect.height() + ejectIconSize.height()) / 2),
                                        QIcon::fromTheme("media-eject-symbolic").pixmap(ejectIconSize));
                    isEjectable = true;
                }
            }
        }
    }

    //Draw item text
    painter->setPen(separatorItem ? Qt::gray : qApp->palette().color(QPalette::ColorRole::Text));
    if (selected)
        painter->setPen("#ffffff");

    QString text = index.data().toString();
    qreal baseValue = itemRect.width() - iconSize.width() - 2 * kItemMargin;
    qreal min = baseValue - 2 * ejectIconSize.width();
    qreal max = baseValue - ejectIconSize.width();

    if (metricsLabel.width(text) > (isEjectable ? min : max))
        text = QFontMetrics(option.widget->font()).elidedText(text, Qt::ElideRight, (isEjectable ? int(min) : int(max)));
    int rowHeight = itemRect.height();
    qreal txtDx = separatorItem ? 22 : 42;
    qreal txtDy = (itemRect.height() - metricsLabel.lineSpacing()) / 2;
    painter->drawText(QRectF(itemRect.x() + txtDx, itemRect.y() + txtDy, itemRect.width(), rowHeight), Qt::AlignLeft, text);
    painter->restore();
}

QSize SideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return DStyledItemDelegate::sizeHint(option, index);
}

void SideBarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Q_UNUSED(model);
    QByteArray n = editor->metaObject()->userProperty().name();
    if (!n.isEmpty()) {
        Q_EMIT rename(index, editor->property(n).toString());
    }

    return;
}

QWidget *SideBarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!this->parent())
        return nullptr;
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    if (!sidebarView)
        return nullptr;
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(sidebarView->model());
    if (!sidebarModel)
        return nullptr;
    SideBarItem *tgItem = sidebarModel->itemFromIndex(index);
    if (!tgItem)
        return nullptr;
    auto sourceInfo = InfoFactory::create<AbstractFileInfo>(tgItem->url());
    if (!sourceInfo)
        return nullptr;
    if (!sourceInfo->exists())
        return nullptr;
    QWidget *editor = DStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit *qle = nullptr;
    if ((qle = dynamic_cast<QLineEdit *>(editor))) {
        QRegularExpression regx(GlobalPrivate::kRegPattern);
        QValidator *validator = new QRegularExpressionValidator(regx, qle);
        qle->setValidator(validator);

        const QString &fs = sourceInfo->extraProperties()["fsType"].toString();
        // For normal file system, the max inputting length is 40. For vfat exfat ，the max value is 11 c
        int maxLen = fs.toLower().endsWith("fat") ? 11 : 40;
        qle->setMaxLength(maxLen);
    }

    return editor;
}

void SideBarItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    DStyledItemDelegate::updateEditorGeometry(editor, option, index);
    //When DTK calculates the width of editor, it does not care about the icon width, so adjust the width of editor here.
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    editor->setFixedWidth(sidebarView->width() - 50);
    QRect rect = editor->geometry();
    rect.setHeight(rect.height() + 2);
    rect.moveTo(40, rect.top());
    editor->setGeometry(rect);
}

bool SideBarItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.isValid()) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            if (e->button() == Qt::LeftButton) {
                QStandardItem *item = qobject_cast<const SideBarModel *>(model)->itemFromIndex(index);
                SideBarItem *sidebarItem = static_cast<SideBarItem *>(item);
                SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);
                bool ejectable = false;
                if (sidebarItem) {
                    ItemInfo info = sidebarItem->itemInfo();
                    ejectable = info.isEjectable;
                }
                QRect expandBtRect(option.rect.width() - 40, option.rect.topRight().y() + 4, 24, 24);
                QRect ejectBtRect(option.rect.bottomRight() + QPoint(-28, -26), option.rect.bottomRight() + QPoint(-kItemMargin, -kItemMargin));
                QPoint pos = e->pos();
                if (separatorItem && expandBtRect.contains(pos)) {   //The expand/unexpand icon is pressed.
                    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
                    if (sidebarView)
                        Q_EMIT changeExpandState(index, !sidebarView->isExpanded(index));

                    event->accept();
                    return true;
                } else if (ejectable && ejectBtRect.contains(pos)) {   //The eject icon is pressed.
                    if (sidebarItem) {
                        QUrl url = sidebarItem->itemInfo().url;
                        SideBarEventCaller::sendEject(url);
                        //onItemActived() slot function would be triggered with mouse clicking,
                        //in order to avoid mount device again, we set item action to disable state as a mark.
                        DViewItemActionList list = sidebarItem->actionList(Qt::RightEdge);
                        if (list.count() > 0)
                            list.first()->setDisabled(true);
                    }
                    event->accept();
                    return true;
                }
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
