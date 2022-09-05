// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsidebardefaultitemhandler.h"

#include "dfmsidebaritem.h"

#include "singleton.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "controllers/pathmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "trashmanager.h"
#include "durl.h"
#include "interfaces/dfilemenu.h"

DFM_BEGIN_NAMESPACE

DFMSideBarItem *DFMSideBarDefaultItemHandler::createItem(const QString &pathKey)
{
    QString iconName = systemPathManager->getSystemPathIconName(pathKey);
    if (!iconName.contains("-symbolic")) {
        iconName.append("-symbolic");
    }

    QString pathStr = pathKey == "Trash" ? TRASH_ROOT : systemPathManager->getSystemPath(pathKey);

    DFMSideBarItem * item = new DFMSideBarItem(
                    QIcon::fromTheme(iconName),
                    systemPathManager->getSystemPathDisplayName(pathKey),
                    DUrl::fromUserInput(pathStr)
                );

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDropEnabled);
    item->setData(SIDEBAR_ID_DEFAULT, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    return item;
}

DFMSideBarDefaultItemHandler::DFMSideBarDefaultItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

void DFMSideBarDefaultItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    return DFMSideBarItemInterface::cdAction(sidebar, item);
}

QMenu *DFMSideBarDefaultItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    static QStringList noPropertySchemes = {"usershare", "network"};

//    if (item->url() != DUrl::fromTrashFile("/") && item->url() != DUrl::fromUserInput(QDir::rootPath())
//            && item->text() != systemPathManager->getSystemPathDisplayName("Home")) {
//        return DFMSideBarItemInterface::contextMenu(sidebar, item);
//    }

    DFileMenu *menu = new DFileMenu();
    menu->setAccessibleInfo(AC_FILE_MENU_DEFAULT_SIDEBAR);

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());

    menu->addAction(QObject::tr("Open in new window"), [item]() {
        WindowManager::instance()->showNewWindow(item->url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, item]() {
        wnd->openNewTab(item->url());
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    if (item->text() == systemPathManager->getSystemPathDisplayName("Recent")) {
        QAction *emptyRecentVisits = new QAction(QObject::tr("Clear recent history"), menu);

        connect(emptyRecentVisits, &QAction::triggered, this, []() {
            appController->actionClearRecent();
        });

        menu->addAction(emptyRecentVisits);
    }

    if (item->text() == systemPathManager->getSystemPathDisplayName("Trash")) {
        QAction *emptyTrash = new QAction(QObject::tr("Empty Trash"), menu);
        connect(emptyTrash, &QAction::triggered, this, [this]() {
            appController->actionClearTrash(this);
        });
        emptyTrash->setDisabled(TrashManager::isEmpty());
        menu->addAction(emptyTrash);
    }

    menu->addSeparator();

    if (!noPropertySchemes.contains(item->url().scheme()) &&
            item->text() != systemPathManager->getSystemPathDisplayName("Recent")) {
        QString propertiesStr = QObject::tr("Properties");
        menu->addAction(propertiesStr, [item]() {
            DUrlList list;
            list.append(item->url());
            Singleton<FileSignalManager>::instance()->requestShowPropertyDialog(DFMUrlListBaseEvent(nullptr, list));
        });
    }

    return menu;
}

DFM_END_NAMESPACE
