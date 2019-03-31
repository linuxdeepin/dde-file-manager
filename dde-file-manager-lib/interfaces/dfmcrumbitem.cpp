/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbitem.h"
#include "dfmcrumbinterface.h"
#include "dfmcrumbbar.h"

#include "dfmapplication.h"
#include "dfileservices.h"
#include "dstorageinfo.h"
#include "dfmsettings.h"

#include "views/dfilemanagerwindow.h"
#include "views/windowmanager.h"
#include "views/themeconfig.h"
#include "deviceinfo/udisklistener.h"

#include <QPainter>
#include <DThemeManager>
#include <QMimeData>

#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>
#include <QMenu>

#include "singleton.h"

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

namespace DFMCrumbItemData {
    static QMap<QString, QIcon> actionIcons;

    void initData();

    void initData()
    {
        // Action Icons:
        if (DFMApplication::genericObtuselySetting()->value("ApplicationAttribute", "DisplayContextMenuIcon", false).toBool()) {
            actionIcons["edit-copy"] = QIcon::fromTheme("edit-copy");
            actionIcons["window-new"] = QIcon::fromTheme("window-new");
            actionIcons["tab-new"] = QIcon::fromTheme("tab-new");
            actionIcons["entry-edit"] = QIcon::fromTheme("entry-edit");
        }
    }
}

class DFMCrumbItemPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbItem)

public:
    DFMCrumbItemPrivate(DFMCrumbItem *qq);

    QPixmap icon() const;
    ThemeConfig::State getState() const;
    QMenu *createStandardContextMenu() const;

    QPoint clickedPos;
    CrumbData data;

    DFMCrumbItem *q_ptr = nullptr;

private:

};

DFMCrumbItemPrivate::DFMCrumbItemPrivate(DFMCrumbItem *qq)
    : q_ptr(qq)
{
    qq->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    qq->setObjectName("DCrumbButton"); // init value, will be changed when setting icon.

    qq->connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, qq, [this, qq](){
        qq->setIcon(icon());
    });
}

QPixmap DFMCrumbItemPrivate::icon() const
{
    return ThemeConfig::instace()->pixmap(data.iconName, data.iconKey, getState());
}

ThemeConfig::State DFMCrumbItemPrivate::getState() const
{
    Q_Q(const DFMCrumbItem);

    if (q->isChecked()) {
        return ThemeConfig::Checked;
    }

    return ThemeConfig::Normal;
}

QMenu *DFMCrumbItemPrivate::createStandardContextMenu() const
{
    Q_Q(const DFMCrumbItem);

    static bool actions_initialized = false;

    if (!actions_initialized) {
        actions_initialized = true;
        DFMCrumbItemData::initData();
    }

    QMenu *menu = new QMenu();

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(q->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());

    menu->addAction(DFMCrumbItemData::actionIcons["edit-copy"], QObject::tr("Copy path"), [this]() {
        QGuiApplication::clipboard()->setText(data.url.toString());
    });

    menu->addAction(DFMCrumbItemData::actionIcons["window-new"], QObject::tr("Open in new window"), [this]() {
        WindowManager::instance()->showNewWindow(data.url, true);
    });

    menu->addAction(DFMCrumbItemData::actionIcons["tab-new"], QObject::tr("Open in new tab"), [wnd, this]() {
        wnd->openNewTab(data.url);
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    menu->addAction(DFMCrumbItemData::actionIcons["entry-edit"], QObject::tr("Edit address"), [=]() {
        DFMCrumbBar* cb_ptr = qobject_cast<DFMCrumbBar*>(q->parentWidget()->parentWidget()->parentWidget()->parentWidget());
        if (cb_ptr) {
            cb_ptr->showAddressBar(qobject_cast<DFileManagerWindow*>(q->topLevelWidget())->currentUrl());
        } else {
            qWarning() << "cannot locate DFMCrumbBar widget to call showAddressBar()";
        }
    });

    return menu;
}

/*!
 * \class DFMCrumbItem
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMCrumbItem is the crumb item inside the crumb bar.
 *
 * DFMCrumbItem is the crumb item which managed by the DFMCrumbBar widget, it holds CrumbData
 * which includes the url of the crumb item, the displaying text, and the icon of the item
 * (if have)
 *
 * \sa DFMCrumbInterface, DFMCrumbManager, DFMCrumbBar
 */

DFMCrumbItem::DFMCrumbItem(const CrumbData &data, QWidget* parent)
    : QPushButton(parent)
    , d_ptr(new DFMCrumbItemPrivate(this))
{
    d_ptr->data = data;

    if (!data.displayText.isEmpty()) {
        this->setText(data.displayText);
    }

    if (!data.iconName.isEmpty()) {
        this->setIconFromThemeConfig(data.iconName, data.iconKey);
    }

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

DFMCrumbItem::~DFMCrumbItem()
{

}

/*!
 * \brief Get the url of the crumb item.
 *
 * \return url of the crumb item.
 */
DUrl DFMCrumbItem::url() const
{
    Q_D(const DFMCrumbItem);
    return d->data.url;
}

/*!
 * \brief Set the display text of the crumb item.
 *
 * \param text The display text which should be set.
 */
void DFMCrumbItem::setText(const QString &text)
{
    Q_D(DFMCrumbItem);
    d->data.displayText = text;
    QPushButton::setText(text);
}

/*!
 * \brief Set the url of the crumb item.
 *
 * \param url The url to be set as.
 */
void DFMCrumbItem::setUrl(const DUrl &url)
{
    Q_D(DFMCrumbItem);
    d->data.url = url;
}

/*!
 * \brief Set icon from theme config.
 *
 * \param group Item group name in theme config file
 * \param key Icon key in theme config file
 *
 * When \a group is empty, item will display as non-icon crumb button.
 */
void DFMCrumbItem::setIconFromThemeConfig(const QString &group, const QString &key)
{
    Q_D(DFMCrumbItem);

    d->data.iconName = group;
    d->data.iconKey = key;

    // Object name and styling should ask design to confirm details
    setObjectName(d->data.iconName.isEmpty() ? "DCrumbButton" : "DCrumbIconButton");

    // Do widget UI update.
    this->setIcon(d->icon());
}

Qt::DropAction DFMCrumbItem::canDropMimeData(const QMimeData *data, Qt::DropActions actions) const
{
    Q_D(const DFMCrumbItem);

    if (data->urls().empty()) {
        return Qt::IgnoreAction;
    }

    for (const DUrl &url : data->urls()) {
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, url);
        if (!fileInfo || !fileInfo->isReadable()) {
            return Qt::IgnoreAction;
        }
    }

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, d->data.url);

    if (!info || !info->canDrop()) {
        return Qt::IgnoreAction;
    }

    const Qt::DropActions support_actions = info->supportedDropActions() & actions;

    if (DStorageInfo::inSameDevice(data->urls().first(), d->data.url)) {
        if (support_actions.testFlag(Qt::MoveAction)) {
            return Qt::MoveAction;
        }
    }

    if (support_actions.testFlag(Qt::CopyAction)) {
        return Qt::CopyAction;
    }

    if (support_actions.testFlag(Qt::MoveAction)) {
        return Qt::MoveAction;
    }

    if (support_actions.testFlag(Qt::LinkAction)) {
        return Qt::LinkAction;
    }

    return Qt::IgnoreAction;
}

bool DFMCrumbItem::dropMimeData(const QMimeData *data, Qt::DropAction action) const
{
    Q_D(const DFMCrumbItem);

    DUrl destUrl = d->data.url;
    DUrlList oriUrlList = DUrl::fromQUrlList(data->urls());
    const DAbstractFileInfoPointer &destInfo = DFileService::instance()->createFileInfo(this, destUrl);

    // convert destnation url to real path if it's a symbol link.
    if (destInfo->isSymLink()) {
        destUrl = destInfo->rootSymLinkTarget();
    }

    switch (action) {
    case Qt::CopyAction:
        DFileService::instance()->pasteFile(this, DFMGlobal::CopyAction, destUrl, oriUrlList);
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        DFileService::instance()->pasteFile(this, DFMGlobal::CutAction, destUrl, oriUrlList);
        break;
    default:
        return false;
    }

    return true;
}

void DFMCrumbItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbItem);
    d->clickedPos = event->globalPos();

    return;
}

void DFMCrumbItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbItem);

    const QPoint pos_difference = d->clickedPos - event->globalPos();

    if (qAbs(pos_difference.x()) < 2 && qAbs(pos_difference.y() < 2)
            && !d->data.url.isEmpty()) {
        click();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void DFMCrumbItem::contextMenuEvent(QContextMenuEvent *event)
{
    Q_D(DFMCrumbItem);

    QMenu *menu = d->createStandardContextMenu();
    menu->exec(event->globalPos());
    menu->deleteLater();
}

void DFMCrumbItem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!isChecked()) {
        double w = width();
        double h = height();
        painter.setPen(QPen(QColor(0, 0, 0, 24), 1));
        painter.drawLine(QPoint(w, 2), QPoint(w, h - 2));
    }

    QPushButton::paintEvent(event);
}

DFM_END_NAMESPACE
