/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "dfmsidebaritem.h"
#include "dfileservices.h"
#include "dfilemenu.h"
#include "dfilemenumanager.h"

#include "views/windowmanager.h"
#include "views/themeconfig.h"

#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <DSvgRenderer>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

#define SIDEBAR_ITEM_WIDTH 201
#define SIDEBAR_ITEM_HEIGHT 30
#define SIDEBAR_ICON_SIZE 16

class DFMSideBarItemPrivate
{
    Q_DECLARE_PUBLIC(DFMSideBarItem)

public:
    DFMSideBarItemPrivate(DFMSideBarItem *qq);
    void init();
    QPixmap icon() const;
    QPixmap reorderLine() const;
    ThemeConfig::State getState() const;

    bool reorderable = false;
    bool readOnly = true;

    DUrl url;
    QFont font;
    QWidget *contentWidget = nullptr;
    QString displayText = "Placeholder";

    DFMSideBarItem *q_ptr = nullptr;

private:
    bool pressed = false;
    bool checked = false;
    bool hovered = false;
    QString iconGroup, iconKey; // use `icon()` and you'll get proper QPixmap for drawing.
};

DFMSideBarItemPrivate::DFMSideBarItemPrivate(DFMSideBarItem *qq)
    : q_ptr(qq)
{
    Q_Q(DFMSideBarItem);
}

void DFMSideBarItemPrivate::init()
{
    Q_Q(DFMSideBarItem);

    q->setAcceptDrops(true);
    q->setMinimumSize(SIDEBAR_ITEM_WIDTH, SIDEBAR_ITEM_HEIGHT);
    q->setIconFromThemeConfig("BookmarkItem.BookMarks", "icon"); // Default icon
    // this approach seems bad, maybe manually set the name is a better idea.
    // and seems DUrl should add a method to generate standard urls directly by a name or enum.
    DAbstractFileInfoPointer file_info = DFileService::instance()->createFileInfo(q, url);
    if (file_info) {
        displayText = file_info->fileDisplayName();
    }
}

QPixmap DFMSideBarItemPrivate::icon() const
{
    return ThemeConfig::instace()->pixmap(iconGroup, iconKey, getState());
}

QPixmap DFMSideBarItemPrivate::reorderLine() const
{
    DSvgRenderer renderer;

    renderer.load(QStringLiteral(":/icons/images/icons/reordering_line.svg"));

    QPainter painter;
    QImage image = QImage(200,
                          renderer.defaultSize().height() * 200.0 / renderer.defaultSize().width(),
                          QImage::Format_ARGB32);

    image.fill(Qt::transparent);
    painter.begin(&image);
    renderer.render(&painter, QRect(QPoint(0, 0), image.size()));
    painter.end();

    return QPixmap::fromImage(image);
}

ThemeConfig::State DFMSideBarItemPrivate::getState() const
{
    if (checked) {
        return ThemeConfig::Checked;
    }

    if (pressed) {
        return ThemeConfig::Pressed;
    }

    if (hovered) {
        return ThemeConfig::Hover;
    }

    return ThemeConfig::Normal;
}

DFMSideBarItem::DFMSideBarItem(const DUrl &url, QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DFMSideBarItemPrivate(this))
{
    d_func()->url = url;

    d_func()->init();
}

DFMSideBarItem::~DFMSideBarItem()
{

}

const DUrl DFMSideBarItem::url() const
{
    Q_D(const DFMSideBarItem);

    return d->url;
}

bool DFMSideBarItem::reorderable() const
{
    Q_D(const DFMSideBarItem);

    return d->reorderable;
}

bool DFMSideBarItem::readOnly() const
{
    Q_D(const DFMSideBarItem);

    return d->readOnly;
}

bool DFMSideBarItem::checked() const
{
    Q_D(const DFMSideBarItem);

    return d->checked;
}

QString DFMSideBarItem::text() const
{
    Q_D(const DFMSideBarItem);

    return d->displayText;
}

void DFMSideBarItem::setContentWidget(QWidget *widget)
{
    Q_D(DFMSideBarItem);

    d->contentWidget = widget;
    // FIXME: attach this widget
}

QWidget *DFMSideBarItem::contentWidget() const
{
    Q_D(const DFMSideBarItem);

    return d->contentWidget;
}

void DFMSideBarItem::setIconFromThemeConfig(const QString &group, const QString &key)
{
    Q_D(DFMSideBarItem);

    d->iconGroup = group;
    d->iconKey = key;

    // Do widget UI update.
    update();
}

void DFMSideBarItem::setReorderable(bool reorderable)
{
    Q_D(DFMSideBarItem);

    d->reorderable = reorderable;
}

void DFMSideBarItem::setReadOnly(bool readOnly)
{
    Q_D(DFMSideBarItem);

    d->readOnly = readOnly;
}

void DFMSideBarItem::setChecked(bool checked)
{
    Q_D(DFMSideBarItem);

    d->checked = checked;

    // Do widget UI update.
    update();
}

void DFMSideBarItem::setText(QString text)
{
    Q_D(DFMSideBarItem);

    d->displayText = text;

    // Do widget UI update.
    update();
}

void DFMSideBarItem::playAnimation()
{

}

QMenu *DFMSideBarItem::createStandardContextMenu() const
{
    QMenu *menu = new QMenu();

    menu->addAction(QObject::tr("Open in new window"));
    menu->addAction(QObject::tr("Open in new tab"));
    menu->addAction(QObject::tr("Properties"));

    return menu;
}

bool DFMSideBarItem::canDropMimeData(const QMimeData *data, Qt::DropAction action) const
{

}

bool DFMSideBarItem::dropMimeData(const QMimeData *data, Qt::DropAction action) const
{

}

void DFMSideBarItem::dragEnterEvent(QDragEnterEvent *event)
{
    Q_D(DFMSideBarItem);

    if (event->source() == this) {
        return;
    }

    if (DFMSideBarItem *item = qobject_cast<DFMSideBarItem *>(event->source())) {
        // FIXME: it's not the right way to check draging is inside the same group
        if (d->reorderable && this->parent() == item->parent()) {
            event->acceptProposedAction();
        }
    }

    if (event->isAccepted()) {
        d->hovered = true;
        update();
    }
}

void DFMSideBarItem::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    Q_D(DFMSideBarItem);

    d->hovered = false;
    update();
}

void DFMSideBarItem::enterEvent(QEvent *event)
{
    Q_D(DFMSideBarItem);

    d->pressed = false; // clear click state
    d->hovered = true;
    update();
}

void DFMSideBarItem::leaveEvent(QEvent *event)
{
    Q_D(DFMSideBarItem);

    d->pressed = false; // only able to press when mouse is on this widget
    d->hovered = false;
    update();
}

void DFMSideBarItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(DFMSideBarItem);

    if (d->pressed && reorderable()) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        drag->setPixmap(d->reorderLine());
        drag->setHotSpot(QPoint(event->x(), 4));
        drag->setMimeData(mimeData);
        drag->exec();
        drag->deleteLater();
    }

    return;
}

void DFMSideBarItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMSideBarItem);

    if (event->button() == Qt::MouseButton::LeftButton) {
        d->pressed = true;
    }

    update();
    return QWidget::mousePressEvent(event);
}

void DFMSideBarItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMSideBarItem);

    if (event->button() == Qt::MouseButton::LeftButton && d->pressed) {
        d->pressed = false;
        emit clicked(); // don't set d->checked when clicked, wait for a signal.
    }

    if (event->button() == Qt::MouseButton::RightButton) {
        QMenu *menu = createStandardContextMenu();
        menu->popup(event->globalPos());
    }

    update();
    return QWidget::mouseReleaseEvent(event);
}

void DFMSideBarItem::paintEvent(QPaintEvent *event)
{
    Q_D(const DFMSideBarItem);
    Q_UNUSED(event);

    QPainter painter(this);

    // Const variables
    const int paddingLeft = 13;
    const int textPaddingLeft = paddingLeft + 24;
    int iconPaddingTop = (height() - SIDEBAR_ICON_SIZE) / 2;
    QColor backgroundColor, textPenColor, iconBrushColor;
    QRect iconRect(paddingLeft, iconPaddingTop, SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    QRect textRect(textPaddingLeft, 0, width() - textPaddingLeft, height());

    backgroundColor = ThemeConfig::instace()->color("BookmarkItem", "background", d->getState());
    textPenColor = ThemeConfig::instace()->color("BookmarkItem", "color", d->getState());
    iconBrushColor = ThemeConfig::instace()->color("BookmarkItem", "background", d->getState()); // what color?

    // Draw Background
    painter.fillRect(rect(), QBrush(backgroundColor));

    // Draw Icon
    painter.setBrush(iconBrushColor);
    painter.drawPixmap(iconRect, d->icon());

    // Draw Text
    painter.setPen(textPenColor);
    QFontMetrics metrics(d->font);
    QString elidedText = metrics.elidedText(d->displayText, Qt::ElideMiddle, width() - textPaddingLeft - 60);
    painter.drawText(textRect, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignVCenter, elidedText);
}

DFM_END_NAMESPACE
