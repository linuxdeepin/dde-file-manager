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
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "views/dtagactionwidget.h"
#include "interfaces/dfmsidebaritem.h"
#include "dfmeventdispatcher.h"
#include "dfmsidebarmanager.h"

#include <QWidgetAction>
#include "durl.h"
#include "dfilemenu.h"

DFM_BEGIN_NAMESPACE

// <TagColorName, ThemeName>
static const QMap<QString, QString> TagColorThemeIconMap {
    {"Orange", "dfm_tag_orange"},
    {"Red", "dfm_tag_red"},
    {"Purple", "dfm_tag_purple"},
    {"Navy-blue", "dfm_tag_deepblue"},
    {"Azure", "dfm_tag_lightblue"},
    {"Grass-green", "dfm_tag_green"},
    {"Yellow", "dfm_tag_yellow"},
    {"Gray", "dfm_tag_gray"}
};

DFMSideBarItem *DFMSideBarTagItemHandler::createItem(const DUrl &url)
{
    QString colorName = TagManager::instance()->getTagColorName(url.fileName());
    QIcon icon = QIcon::fromTheme(TagColorThemeIconMap[colorName]);
    DFMSideBarItem * item = new DFMSideBarItem(icon, url.fileName(), url);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
    item->setData(SIDEBAR_ID_TAG, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    return item;
}

DFMSideBarTagItemHandler::DFMSideBarTagItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

QMenu *DFMSideBarTagItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
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
#ifdef QT_DEBUG // fixme: implement rename
    menu->addAction(QObject::tr("Rename"), [sidebar, item]() {
        int index = sidebar->findItem(item);
        if (index >= 0) {
            sidebar->openItemEditor(index);
        }
    });
#endif // QT_DEBUG
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

DFM_END_NAMESPACE
