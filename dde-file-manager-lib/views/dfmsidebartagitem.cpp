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
#include "dfilemanagerwindow.h"
#include "dfmsidebartagitem.h"
#include "dtagactionwidget.h"
#include "dfilemenu.h"
#include "dfmeventdispatcher.h"
#include "dfmevent.h"

#include "tag/tagmanager.h"
#include "views/windowmanager.h"

#include <QWidgetAction>

DFM_BEGIN_NAMESPACE

DFMSideBarTagItem::DFMSideBarTagItem(const DUrl &url, QWidget *parent)
    : DFMSideBarItem(url, parent)
{
    setCanDeleteViaDrag(true);
    setReorderable(true);
    QString colorName = TagManager::instance()->getTagColorName(url.fileName());
    setIconFromThemeConfig("BookmarkItem." + colorName);
}

QMenu *DFMSideBarTagItem::createStandardContextMenu() const
{
    DFileMenu *menu = new DFileMenu();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    DTagActionWidget* tagWidget{ new DTagActionWidget };
    QWidgetAction* tagAction{ new QWidgetAction{ nullptr } };

    menu->addAction(QObject::tr("Open in new window"), [this]() {
        WindowManager::instance()->showNewWindow(url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this]() {
        wnd->openNewTab(url());
    })->setDisabled(shouldDisable);

    menu->addAction(QObject::tr("Rename"), [this]() {
        DFMSideBarTagItem *ccItem = const_cast<DFMSideBarTagItem *>(this);
        ccItem->showRenameEditor();
    });

    menu->addAction(QObject::tr("Remove"), [ = ]() {
        DFileService::instance()->deleteFiles(this, DUrlList{url()}, false);
    });

    tagAction->setDefaultWidget(tagWidget);
    tagAction->setText("Change color of present tag");
    tagWidget->setExclusive(true);
    tagWidget->setToolTipVisible(false);

    menu->addAction(tagAction);
    connect(tagAction, &QWidgetAction::triggered, this, [this, menu]() {
        DFMEventDispatcher::instance()->processEvent<DFMMenuActionEvent>(this, menu, DUrl(TAG_ROOT),
        DUrlList{this->url()}, DFMGlobal::ChangeTagColor);
    });

    return menu;
}

// dtagactionwidget

DFM_END_NAMESPACE
