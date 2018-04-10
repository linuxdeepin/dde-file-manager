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

#include <QPainter>

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
    ThemeConfig::State getState() const;

    bool hasDrag = false;
    bool readOnly = true;

    DUrl url;
    QFont font;
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

    q->installEventFilter(q);
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

bool DFMSideBarItem::hasDrag() const
{
    Q_D(const DFMSideBarItem);

    return d->hasDrag;
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

void DFMSideBarItem::setIconFromThemeConfig(const QString &group, const QString &key)
{
    Q_D(DFMSideBarItem);

    d->iconGroup = group;
    d->iconKey = key;

    // Do widget UI update.
    update();
}

void DFMSideBarItem::setHasDrag(bool hasDrag)
{
    Q_D(DFMSideBarItem);

    d->hasDrag = hasDrag;
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

/*
    TODO: Should create some new classes which extend `DFMSideBarItem`
    and overwrite this function to get different context menu instead
    of checking type or other attribs.
*/
QMenu *DFMSideBarItem::createStandardContextMenu() const
{
    Q_D(const DFMSideBarItem);

    // Thing: this acutally return a `DFileMenu` rather than a `DFileMenu`.
    DFileMenu *menu = 0;
    QSet<MenuAction> disableList;

    const bool tabAddable = true;// check out how to do this
    if (!tabAddable || (d->url.isLocalFile() && !QFile::exists(d->url.toLocalFile()))) {
        disableList << MenuAction::OpenInNewTab;
    }

    if (d->url.isRecentFile()) {
        menu = DFileMenuManager::createRecentLeftBarMenu(disableList);
    } else if (d->url.isTrashFile()) {
        menu = DFileMenuManager::createTrashLeftBarMenu(disableList);
    } else if (d->url.isComputerFile()) {
        menu = DFileMenuManager::createComputerLeftBarMenu(disableList);
    }/* else if (m_isDisk && m_deviceInfo) {
        if (!tabAddable) {
            disableList << MenuAction::OpenDiskInNewTab;
        }

        disableList |= m_deviceInfo->disableMenuActionList() ;
        m_url.setQuery(m_deviceID);

        menu = DFileMenuManager::genereteMenuByKeys(
                   m_deviceInfo->menuActionList(DAbstractFileInfo::SingleFile),
                   disableList);
    } */else if (d->url.isNetWorkFile()) {
        menu = DFileMenuManager::createNetworkMarkMenu(disableList);
    } else if (d->url.isUserShareFile()) {
        menu = DFileMenuManager::createUserShareMarkMenu(disableList);
    }/* else if (PluginManager::instance()->getViewInterfaceByScheme(d->url.scheme())) {
        menu = DFileMenuManager::createPluginBookMarkMenu(disableList);
    } else if (d->m_isDefault) {
        menu = DFileMenuManager::createDefaultBookMarkMenu(disableList);

        ///###: tag protocol.
    } else if (d->url.isTagedFile()) {
        menu = DFileMenuManager::createTagMarkMenu(disableList);
        //DBookmarkItem::ClickedItem = this; ???

    }*/ else {
        menu = DFileMenuManager::createCustomBookMarkMenu(d->url, disableList);
    }
    return menu;
}

bool DFMSideBarItem::canDropMimeData(const QMimeData *data, Qt::DropAction action) const
{

}

bool DFMSideBarItem::dropMimeData(const QMimeData *data, Qt::DropAction action) const
{

}

bool DFMSideBarItem::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DFMSideBarItem);

    // Only care about events of current widget
    if (watched != this) {
        return false;
    }

    // Hover
    if (event->type() == QEvent::Enter) {
        d->pressed = false; // clear click state
        d->hovered = true;
        update();
    } else if (event->type() == QEvent::Leave) {
        d->pressed = false; // only able to press when mouse is on this widget
        d->hovered = false;
        update();
    }

    // Return: Event filters do NOT allow further processing of the event?
    return QWidget::eventFilter(watched, event);
}

void DFMSideBarItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    // nothing for now, just ate mouse move event.

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
