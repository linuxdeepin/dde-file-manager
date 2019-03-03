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
#include "dfmsidebartrashitem.h"

#include <QMenu>
#include <QAction>

#include "dfilemanagerwindow.h"
#include "singleton.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "views/windowmanager.h"
#include "controllers/appcontroller.h"
#include "controllers/trashmanager.h"

DFM_BEGIN_NAMESPACE

DFMSideBarTrashItem::DFMSideBarTrashItem()
    : DFMSideBarDefaultItem(DFMStandardPaths::StandardLocation::TrashPath)
{

}

QMenu *DFMSideBarTrashItem::createStandardContextMenu() const
{
    QMenu *menu = new QMenu();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());

    menu->addAction(QObject::tr("Open in new window"), [this]() {
        WindowManager::instance()->showNewWindow(url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this]() {
        wnd->openNewTab(url());
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    QAction *emptyTrash = new QAction(QObject::tr("Empty Trash"), menu);
    connect(emptyTrash, &QAction::triggered, this, [this]() {
        appController->actionClearTrash(this);
    });
    emptyTrash->setDisabled(TrashManager::isEmpty());
    menu->addAction(emptyTrash);

    menu->addSeparator();

    menu->addAction(QObject::tr("Properties"), [this]() {
        DUrlList list;
        list.append(url());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, list));
    });

    return menu;
}

DFM_END_NAMESPACE
