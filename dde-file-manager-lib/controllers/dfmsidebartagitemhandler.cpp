/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
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

#include "dfmsidebartagitemhandler.h"

#include "tag/tagmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmleftsidebar.h"
#include "views/windowmanager.h"
#include "views/dtagactionwidget.h"
#include "interfaces/dfmleftsidebaritem.h"
#include "dfmeventdispatcher.h"
#include "dfmsidebarmanager.h"

#include <QWidgetAction>
#include "durl.h"
#include "dfilemenu.h"

// <TagColorName, ThemeName>
static const QMap<QString, QString> TagColorThemeIconMap {
    {"Orange", "dfm_tag-orange"},
    {"Red", "dfm_tag-red"},
    {"Purple", "dfm_tag-purple"},
    {"Navy-blue", "dfm_tag-deepblue"},
    {"Azure", "dfm_tag-lightblue"},
    {"Grass-green", "dfm_tag-green"},
    {"Yellow", "dfm_tag-yellow"},
    {"Gray", "dfm_tag-gray"}
};

DFMLeftSideBarItem *DFMSideBarTagItemHandler::createItem(const DUrl &url)
{
    QString colorName = TagManager::instance()->getTagColorName(url.fileName());
    QIcon icon = QIcon::fromTheme(TagColorThemeIconMap[colorName]);
    DFMLeftSideBarItem * item = new DFMLeftSideBarItem(icon, url.fileName(), url);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
    item->setData(SIDEBAR_ID_TAG, DFMLeftSideBarItem::ItemUseRegisteredHandlerRole);

    return item;
}

DFMSideBarTagItemHandler::DFMSideBarTagItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

QMenu *DFMSideBarTagItemHandler::contextMenu(const DFMLeftSideBar *sidebar, const DFMLeftSideBarItem *item)
{
    DFileMenu *menu = new DFileMenu();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    DTagActionWidget* tagWidget{ new DTagActionWidget };
    QWidgetAction* tagAction{ new QWidgetAction{ nullptr } };

    menu->addAction(QObject::tr("Open in new window"), [item]() {
        WindowManager::instance()->showNewWindow(item->url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, item]() {
        wnd->openNewTab(item->url());
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    menu->addAction(QObject::tr("Rename"), [sidebar, item]() {
        int index = sidebar->findItem(item);
        if (index >= 0) {
            sidebar->openItemEditor(index);
        }
    });

    menu->addAction(QObject::tr("Remove"), [item]() {
        DFileService::instance()->deleteFiles(nullptr, DUrlList{item->url()}, false);
    });

    menu->addSeparator();

    tagAction->setDefaultWidget(tagWidget);
    tagAction->setText("Change color of present tag");
    tagWidget->setExclusive(true);
    tagWidget->setToolTipVisible(false);

    menu->addAction(tagAction);
    connect(tagAction, &QWidgetAction::triggered, this, [item, menu]() {
        DFMEventDispatcher::instance()->processEvent<DFMMenuActionEvent>(nullptr, menu, DUrl(TAG_ROOT),
        DUrlList{item->url()}, DFMGlobal::ChangeTagColor);
    });

    return menu;
}
