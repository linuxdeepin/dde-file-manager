/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dfilemenumanager.h"
#include "dfmglobal.h"
#include "app/define.h"
#include "dfmevent.h"
#include "dfilemenu.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"
#include "controllers/appcontroller.h"
#include "controllers/trashmanager.h"
#include "models/desktopfileinfo.h"
#include "shutil/mimetypedisplaymanager.h"

#include "singleton.h"
#include "views/windowmanager.h"
#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "controllers/pathmanager.h"
#include "plugins/pluginmanager.h"
#include "dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h"
#include "dfmstandardpaths.h"
#include "deviceinfo/udisklistener.h"
#include "views/dtagactionwidget.h"

#include <QMetaObject>
#include <QMetaEnum>
#include <QMenu>
#include <QSettings>
#include <QTextCodec>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QQueue>
#include <QDebug>
#include <QPushButton>
#include <QWidgetAction>


namespace DFileMenuData {
static QMap<MenuAction, QString> actionKeys;
static QMap<MenuAction, QAction*> actions;
static QMap<const QAction*, MenuAction> actionToMenuAction;
static QMap<MenuAction, QString> actionIDs;
static QVector<MenuAction> sortActionTypes;
static QSet<MenuAction> whitelist;
static QSet<MenuAction> blacklist;
static QQueue<MenuAction> availableUserActionQueue;

void initData();
void initActions();

MenuAction takeAvailableUserActionType()
{
    if (availableUserActionQueue.isEmpty()) {
        availableUserActionQueue.append(MenuAction(MenuAction::UserMenuAction + 1));

        return MenuAction::UserMenuAction;
    }

    MenuAction type = availableUserActionQueue.dequeue();

    if (availableUserActionQueue.isEmpty())
        availableUserActionQueue.append(MenuAction(type + 1));

    return type;
}
void recycleUserActionType(MenuAction type)
{
    availableUserActionQueue.prepend(type);
    QAction *action = actions.take(type);

    if (action)
        actionToMenuAction.remove(action);
}
}

DFileMenu *DFileMenuManager::createRecentLeftBarMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab
               << MenuAction::ClearRecent
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createDefaultBookMarkMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createNetworkMarkMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;;

    actionKeys.reserve(1);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createPluginBookMarkMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;;

    actionKeys.reserve(1);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createUserShareMarkMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;;

    actionKeys.reserve(1);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createCustomBookMarkMenu(const DUrl &url, QSet<MenuAction> disableList)
{

    QVector<MenuAction> actionKeys;

    actionKeys.reserve(10);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab
               << MenuAction::BookmarkRename
               << MenuAction::BookmarkRemove
               << MenuAction::Property;

    const DAbstractFileInfoPointer& info = fileService->createFileInfo(Q_NULLPTR, url);
    info->refresh();
    if (!info->exists()){
        disableList << MenuAction::OpenInNewWindow
                    << MenuAction::OpenInNewTab
                    << MenuAction::BookmarkRename
                    << MenuAction::Property;
    }

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createTrashLeftBarMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::OpenInNewWindow;
    actionKeys << MenuAction::OpenInNewTab;
    actionKeys << MenuAction::ClearTrash;
    actionKeys<< MenuAction::Property;

    if (TrashManager::isEmpty()){
        QSet<MenuAction> tmp_disableList = disableList;

        tmp_disableList << MenuAction::ClearTrash;

        return genereteMenuByKeys(actionKeys, tmp_disableList);
    }

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createComputerLeftBarMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << MenuAction::OpenInNewWindow;
    actionKeys << MenuAction::OpenInNewTab;
    actionKeys << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createDiskLeftBarMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Open
               << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createDiskViewMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << MenuAction::Open << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Mount << MenuAction::Unmount
               << MenuAction::Separator
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createToolBarSettingsMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;
    QMap<MenuAction, QVector<MenuAction> >  subMenuKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::NewWindow
               << MenuAction::ToggleTheme
               << MenuAction::Separator
               << MenuAction::SetUserSharePassword
               << MenuAction::Settings;

    return genereteMenuByKeys(actionKeys, disableList, false, subMenuKeys, false);
}

DFileMenu *DFileMenuManager::createToolBarSortMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::Name << MenuAction::Size
               << MenuAction::Type
               << MenuAction::CreatedDate
               << MenuAction::LastModifiedDate;
    DFileMenuData::sortActionTypes = actionKeys;
    DFileMenu *menu = genereteMenuByKeys(actionKeys, disableList, true);
    return menu;
}

DFileMenu *DFileMenuManager::createListViewHeaderMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Size
               << MenuAction::Type
               << MenuAction::CreatedDate
               << MenuAction::LastModifiedDate;

    DFileMenu *menu = genereteMenuByKeys(actionKeys, disableList, true);
    return menu;
}

DFileMenu *DFileMenuManager::createTagMarkMenu(const QSet<MenuAction>& disableList)
{
    QVector<MenuAction> actionKeys;
    actionKeys.push_back(MenuAction::OpenInNewWindow);
    actionKeys.push_back(MenuAction::OpenInNewTab);
    actionKeys.push_back(MenuAction::RenameTag);
    actionKeys.push_back(MenuAction::DeleteTags);
    actionKeys.push_back(MenuAction::Separator);
    actionKeys.push_back(MenuAction::ChangeTagColor);

    DFileMenu* menu{ genereteMenuByKeys(actionKeys, disableList, false) };
    return menu;
}

DFileMenu *DFileMenuManager::createNormalMenu(const DUrl &currentUrl, const DUrlList &urlList, QSet<MenuAction> disableList, QSet<MenuAction> unusedList, int windowId)
{
    DAbstractFileInfoPointer info = fileService->createFileInfo(Q_NULLPTR, currentUrl);
    DFileMenu *menu = Q_NULLPTR;
    if (!info)
        return menu;

    if (urlList.length() == 1) {
        QVector<MenuAction> actions = info->menuActionList(DAbstractFileInfo::SingleFile);

        foreach (MenuAction action, unusedList) {
            if (actions.contains(action)){
                actions.remove(actions.indexOf(action));
            }
        }

        if (actions.isEmpty())
            return menu;

        const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();
        disableList += DFileMenuManager::getDisableActionList(urlList);
        const bool& tabAddable = WindowManager::tabAddableByWinId(windowId);
        if(!tabAddable)
            disableList << MenuAction::OpenInNewTab;

        menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);


        QAction *openWithAction = menu->actionAt(DFileMenuManager::getActionString(DFMGlobal::OpenWith));
        DFileMenu* openWithMenu = openWithAction ? qobject_cast<DFileMenu*>(openWithAction->menu()) : Q_NULLPTR;

        if (openWithMenu) {
            QStringList recommendApps = mimeAppsManager->getRecommendedApps(info->redirectedFileUrl());

            foreach (QString app, recommendApps) {
//                const DesktopFile& df = mimeAppsManager->DesktopObjs.value(app);
                //ignore no show apps
//                if(df.getNoShow())
//                    continue;
                DesktopFile desktopFile(app);
                QAction* action = new QAction(desktopFile.getLocalName(), openWithMenu);
                action->setIcon(FileUtils::searchAppIcon(desktopFile));
                action->setProperty("app", app);
                action->setProperty("url", QVariant::fromValue(info->redirectedFileUrl()));
                openWithMenu->addAction(action);
                connect(action, &QAction::triggered, appController, &AppController::actionOpenFileByApp);
            }

            QAction* action = new QAction(fileMenuManger->getActionString(MenuAction::OpenWithCustom), openWithMenu);
            action->setData((int)MenuAction::OpenWithCustom);
            openWithMenu->addAction(action);
            DFileMenuData::actions[MenuAction::OpenWithCustom] = action;
            DFileMenuData::actionToMenuAction[action] = MenuAction::OpenWithCustom;
        }
    } else {
        bool isSystemPathIncluded = false;
        bool isAllCompressedFiles = true;

        foreach (DUrl url, urlList) {
            const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(Q_NULLPTR, url);

            if(!FileUtils::isArchive(url.path()))
                isAllCompressedFiles = false;

            if (systemPathManager->isSystemPath(fileInfo->fileUrl().toLocalFile())) {
                isSystemPathIncluded = true;
            }
        }

        QVector<MenuAction> actions;

        if (isSystemPathIncluded)
            actions = info->menuActionList(DAbstractFileInfo::MultiFilesSystemPathIncluded);
        else
            actions = info->menuActionList(DAbstractFileInfo::MultiFiles);

        if (actions.isEmpty())
            return menu;

        if(isAllCompressedFiles){
            int index = actions.indexOf(MenuAction::Compress);
            actions.insert(index + 1, MenuAction::Decompress);
            actions.insert(index + 2, MenuAction::DecompressHere);
        }

        const QMap<MenuAction, QVector<MenuAction> >& subActions  = info->subMenuActionList();
        disableList += DFileMenuManager::getDisableActionList(urlList);
        const bool& tabAddable = WindowManager::tabAddableByWinId(windowId);
        if(!tabAddable)
            disableList << MenuAction::OpenInNewTab;

        foreach (MenuAction action, unusedList) {
            if (actions.contains(action)){
                actions.remove(actions.indexOf(action));
            }
        }
        menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    }

    if (deviceListener->isMountedRemovableDiskExits()){
        QAction *sendToMountedRemovableDiskAction = menu->actionAt(DFileMenuManager::getActionString(DFMGlobal::SendToRemovableDisk));

        DFileMenu* sendToMountedRemovableDiskMenu = sendToMountedRemovableDiskAction ? qobject_cast<DFileMenu*>(sendToMountedRemovableDiskAction->menu()) : Q_NULLPTR;
        if (sendToMountedRemovableDiskMenu){
            foreach (UDiskDeviceInfoPointer pDeviceinfo, deviceListener->getCanSendDisksByUrl(currentUrl.toLocalFile()).values()) {
                QAction* action = new QAction(pDeviceinfo->getDiskInfo().name(), sendToMountedRemovableDiskMenu);
                action->setProperty("mounted_root_uri", pDeviceinfo->getDiskInfo().mounted_root_uri());
                action->setProperty("urlList", DUrl::toStringList(urlList));
                sendToMountedRemovableDiskMenu->addAction(action);
                connect(action, &QAction::triggered, appController, &AppController::actionSendToRemovableDisk);
            }
        }
    }


    if(currentUrl == DesktopFileInfo::computerDesktopFileUrl() ||
            currentUrl == DesktopFileInfo::trashDesktopFileUrl())
        return menu;

    loadNormalPluginMenu(menu, urlList, currentUrl);
    loadNormalExtensionMenu(menu, urlList, currentUrl);

    return menu;
}

QList<QAction*> DFileMenuManager::loadNormalPluginMenu(DFileMenu *menu, const DUrlList &urlList, const DUrl &currentUrl)
{
    qDebug() << "load normal plugin menu";
    QStringList files;
    foreach (DUrl url, urlList) {
        files << url.toString();
    }

    QAction* lastAction = menu->actions().last();
    if (lastAction->isSeparator()){
        lastAction = menu->actionAt(menu->actions().count() - 2);
    }

    QList<QAction *> actions;
    foreach (MenuInterface* menuInterface, PluginManager::instance()->getMenuInterfaces()) {
        actions = menuInterface->additionalMenu(files, currentUrl.toString());
        foreach (QAction* action, actions) {
            menu->insertAction(lastAction, action);
        }
    }
    menu->insertSeparator(lastAction);
    return actions;
}

QList<QAction*> DFileMenuManager::loadNormalExtensionMenu(DFileMenu *menu, const DUrlList &urlList, const DUrl &currentUrl)
{
    qDebug() << "load normal extension menu";
    QAction* lastAction = menu->actions().last();
    if (lastAction->isSeparator()){
        lastAction = menu->actionAt(menu->actions().count() - 2);
    }

    QList<QAction *> actions = loadMenuExtemsionActions(urlList, currentUrl);
    foreach (QAction* action, actions) {
        menu->insertAction(lastAction, action);
    }

    menu->insertSeparator(lastAction);
    return actions;
}

QList<QAction*> DFileMenuManager::loadEmptyAreaPluginMenu(DFileMenu *menu, const DUrl &currentUrl)
{
    qDebug() << "load empty area plugin menu";
    QAction* lastAction = menu->actions().last();
    if (lastAction->isSeparator()){
        lastAction = menu->actionAt(menu->actions().count() - 2);
    }

    QList<QAction *> actions;
    foreach (MenuInterface* menuInterface, PluginManager::instance()->getMenuInterfaces()) {
        actions = menuInterface->additionalEmptyMenu(currentUrl.toString());
        foreach (QAction* action, actions) {
            menu->insertAction(lastAction, action);
        }
    }
    menu->insertSeparator(lastAction);
    return actions;
}

QList<QAction *> DFileMenuManager::loadEmptyAreaExtensionMenu(DFileMenu *menu, const DUrl &currentUrl)
{
    qDebug() << "load empty area extension menu";
    QAction* lastAction = menu->actions().last();

    if (lastAction->isSeparator()){
        lastAction = menu->actionAt(menu->actions().count() - 2);
    }

    DUrlList urlList;
    QList<QAction *> actions = loadMenuExtemsionActions(urlList, currentUrl);
    foreach (QAction* action, actions) {
        menu->insertAction(lastAction, action);
    }
    menu->insertSeparator(lastAction);

    return actions;
}

QList<QAction *> DFileMenuManager::loadMenuExtemsionActions(const DUrlList &urlList, const DUrl& currentUrl)
{
    QList<QAction *>  actions;

    QStringList menuExtensionPaths = DFMGlobal::MenuExtensionPaths;

    foreach (QString path, menuExtensionPaths) {
        QFileInfo info(path);
        qDebug() << info.absoluteFilePath();
        QDir menuExtensionDir(path);
        menuExtensionDir.makeAbsolute();

        qDebug() << path << menuExtensionDir.absolutePath();

        DFMGlobal::MenuExtension menuExtensionType = FileUtils::getMenuExtension(urlList);

        QMetaEnum metaEnum = QMetaEnum::fromType<DFMGlobal::MenuExtension>();
        QString menuType = metaEnum.valueToKey(menuExtensionType);

        foreach (QFileInfo fileInfo, menuExtensionDir.entryInfoList(QDir::Files)){
            if (fileInfo.fileName().endsWith(".json")){
                qDebug() << fileInfo.absoluteFilePath();
                QFile file(fileInfo.absoluteFilePath());
                if (!file.open(QIODevice::ReadOnly))
                {
                    qDebug() << "Couldn't open" << fileInfo.absoluteFilePath();
                    return actions;
                }
                QByteArray data = file.readAll();
                QJsonDocument jsonDoc(QJsonDocument::fromJson(data));

                actions = jsonToActions(jsonDoc.array(), urlList, currentUrl, menuType);
            }
        }
    }

    return actions;
}

QList<QAction *> DFileMenuManager::jsonToActions(const QJsonArray& data, const DUrlList &urlList, const DUrl &currentUrl, const QString& menuExtensionType)
{
    QList<QAction *> actions;

    foreach (const QJsonValue& value, data) {
        QJsonObject v = value.toObject();
        QString menuType = v.toVariantMap().value("MenuType").toString();
        QString mimeType = v.toVariantMap().value("MimeType").toString();
        QString suffix = v.toVariantMap().value("Suffix").toString();

        QString icon = v.toVariantMap().value("Icon").toString();

        QString textKey = QString("Text[%1]").arg(QLocale::system().name());
        QString text = v.toVariantMap().value(textKey).toString();

        QString exec = v.toVariantMap().value("Exec").toString();

        QVariantList subMenuDataList = v.toVariantMap().value("SubMenu").toList();

        bool isCanCreateAction = false;

        if (menuType == menuExtensionType){
            isCanCreateAction = true;
        }

        if (isCanCreateAction){
            if (menuExtensionType == "SingleFile" ||
                    menuExtensionType== "MultiFiles"){

                if (mimeType.isEmpty() && suffix.isEmpty()){
                    isCanCreateAction = true;
                }
                if (!mimeType.isEmpty()){
                    QStringList supportMimeTypes = mimeType.split(";");
                    int count = 0;
                    foreach (DUrl url, urlList) {
                        QString mimeType = FileUtils::getFileMimetype(url.toLocalFile());

                        if (supportMimeTypes.isEmpty() || supportMimeTypes.contains(mimeType)){
                            count += 1;
                        }
                    }
                    if (count == urlList.count()){
                        isCanCreateAction = true;
                    }else{
                        isCanCreateAction = false;
                    }
                }
                if (!suffix.isEmpty()){
                    QStringList supportsuffixs = suffix.split(";");
                    int count = 0;
                    foreach (DUrl url, urlList) {
                        QString _suxffix = QFileInfo(url.toLocalFile()).suffix();

                        if (supportsuffixs.isEmpty() || supportsuffixs.contains(_suxffix)){
                            count += 1;
                        }
                    }
                    if (count == urlList.count()){
                        isCanCreateAction = true;
                    }else{
                        isCanCreateAction = false;
                    }
                }
            }
        }

        if (isCanCreateAction){
            QAction* action = new QAction(QIcon(icon), text, NULL);

            if (subMenuDataList.count() > 1){
                QJsonArray subActionsArray;
                QJsonArray _subActionsArray = QJsonArray::fromVariantList(subMenuDataList);
                foreach (QJsonValue v, _subActionsArray) {
                    QJsonObject obj = v.toObject();
                    obj.insert("MenuType", menuType);
                    subActionsArray.append(QJsonValue(obj));
                }
                QList<QAction *> subActions = jsonToActions(subActionsArray, urlList, currentUrl, menuExtensionType);
                QMenu* menu = new QMenu;
                menu->addActions(subActions);
                action->setMenu(menu);
            }else{
                connect(action, &QAction::triggered, [=](){

                    QProcess p;
                    QStringList args;
                    foreach (DUrl url, urlList) {
                        args << url.toString();
                    }

                    if (urlList.isEmpty()){
                        args << currentUrl.toString();
                    }

                    p.startDetached(exec, args);
                    qDebug() << exec << args;
                });
            }
            actions << action;
        }
    }
    return actions;
}

QAction *DFileMenuManager::getAction(MenuAction action)
{
    return DFileMenuData::actions.value(action);
}

QSet<MenuAction> DFileMenuManager::getDisableActionList(const DUrl &fileUrl)
{
    DUrlList list;

    list << fileUrl;

    return getDisableActionList(list);
}

QSet<MenuAction> DFileMenuManager::getDisableActionList(const DUrlList &urlList)
{
    QSet<MenuAction> disableList;

    for (const DUrl &fileUrl : urlList) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(Q_NULLPTR, fileUrl);

        if (fileInfo) {
            disableList += fileInfo->disableMenuActionList();
        }
    }

    if (DFMGlobal::instance()->clipboardAction() == DFMGlobal::UnknowAction){
        disableList << MenuAction::Paste;
    }

    return disableList;
}

DFileMenuManager::DFileMenuManager()
{
    qRegisterMetaType<QMap<QString, QString>>(QT_STRINGIFY(QMap<QString, QString>));
    qRegisterMetaType<QList<QUrl>>(QT_STRINGIFY(QList<QUrl>));
}

void DFileMenuData::initData()
{
    actionKeys[MenuAction::Open] = QObject::tr("Open");
    actionKeys[MenuAction::OpenInNewWindow] = QObject::tr("Open in new window");
    actionKeys[MenuAction::OpenInNewTab] = QObject::tr("Open in new tab");
    actionKeys[MenuAction::OpenDisk] = QObject::tr("Open");
    actionKeys[MenuAction::OpenDiskInNewWindow] = QObject::tr("Open in new window");
    actionKeys[MenuAction::OpenDiskInNewTab] = QObject::tr("Open in new tab");
    actionKeys[MenuAction::OpenAsAdmin] = QObject::tr("Open in new window as admin");
    actionKeys[MenuAction::OpenWith] = QObject::tr("Open with");
    actionKeys[MenuAction::OpenWithCustom] = QObject::tr("Select default program");
    actionKeys[MenuAction::OpenFileLocation] = QObject::tr("Open file location");
    actionKeys[MenuAction::Compress] = QObject::tr("Compress");
    actionKeys[MenuAction::Decompress] = QObject::tr("Extract");
    actionKeys[MenuAction::DecompressHere] = QObject::tr("Extract here");
    actionKeys[MenuAction::Cut] = QObject::tr("Cut");
    actionKeys[MenuAction::Copy] = QObject::tr("Copy");
    actionKeys[MenuAction::Paste] = QObject::tr("Paste");
    actionKeys[MenuAction::Rename] = QObject::tr("Rename");
    actionKeys[MenuAction::BookmarkRename] = QObject::tr("Rename");
    actionKeys[MenuAction::BookmarkRemove] = QObject::tr("Remove");
    actionKeys[MenuAction::CreateSymlink] = QObject::tr("Create link");
    actionKeys[MenuAction::SendToDesktop] = QObject::tr("Send to desktop");
    actionKeys[MenuAction::SendToRemovableDisk] = QObject::tr("Send to");
    actionKeys[MenuAction::AddToBookMark] = QObject::tr("Add to bookmark");
    actionKeys[MenuAction::Delete] = QObject::tr("Delete");
    actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Delete");
    actionKeys[MenuAction::Property] = QObject::tr("Properties");

    actionKeys[MenuAction::NewFolder] = QObject::tr("New folder");
    actionKeys[MenuAction::NewWindow] = QObject::tr("New window");
    actionKeys[MenuAction::SelectAll] = QObject::tr("Select all");
    actionKeys[MenuAction::ClearRecent] = QObject::tr("Clear recent history");
    actionKeys[MenuAction::ClearTrash] = QObject::tr("Empty Trash");
    actionKeys[MenuAction::DisplayAs] = QObject::tr("Display as");
    actionKeys[MenuAction::SortBy] = QObject::tr("Sort by");
    actionKeys[MenuAction::NewDocument] = QObject::tr("New document");
    actionKeys[MenuAction::NewWord] = QObject::tr("Office Text Document");
    actionKeys[MenuAction::NewExcel] = QObject::tr("Spreadsheet Document");
    actionKeys[MenuAction::NewPowerpoint] = QObject::tr("Presentation Document");
    actionKeys[MenuAction::NewText] = QObject::tr("Plain Text Document");
    actionKeys[MenuAction::OpenInTerminal] = QObject::tr("Open in terminal");
    actionKeys[MenuAction::Restore] = QObject::tr("Restore");
    actionKeys[MenuAction::RestoreAll] = QObject::tr("Restore all");
    actionKeys[MenuAction::Mount] = QObject::tr("Mount");
    actionKeys[MenuAction::Unmount]= QObject::tr("Unmount");
    actionKeys[MenuAction::Eject]= QObject::tr("Eject");
    actionKeys[MenuAction::SafelyRemoveDrive]= QObject::tr("Safely Remove");
    actionKeys[MenuAction::Name] = QObject::tr("Name");
    actionKeys[MenuAction::Size] = QObject::tr("Size");
    actionKeys[MenuAction::Type] = QObject::tr("Type");
    actionKeys[MenuAction::CreatedDate] = QObject::tr("Time created");
    actionKeys[MenuAction::LastModifiedDate] = QObject::tr("Time modified");
    actionKeys[MenuAction::Settings] = QObject::tr("Settings");
    actionKeys[MenuAction::Help] = QObject::tr("Help");
    actionKeys[MenuAction::About] = QObject::tr("About");
    actionKeys[MenuAction::Exit] = QObject::tr("Exit");
    actionKeys[MenuAction::IconView] = QObject::tr("Icon");
    actionKeys[MenuAction::ListView] = QObject::tr("List");
    actionKeys[MenuAction::ExtendView] = QObject::tr("Extend");
    actionKeys[MenuAction::SetAsWallpaper] = QObject::tr("Set as wallpaper");
    actionKeys[MenuAction::ForgetPassword] = QObject::tr("Log out and unmount");
    actionKeys[MenuAction::DeletionDate] = QObject::tr("Time deleted");
    actionKeys[MenuAction::SourcePath] = QObject::tr("Source path");
    actionKeys[MenuAction::AbsolutePath] = QObject::tr("Path");
    actionKeys[MenuAction::Share] = QObject::tr("Share folder");
    actionKeys[MenuAction::UnShare] = QObject::tr("Cancel sharing");
    actionKeys[MenuAction::SetUserSharePassword] = QObject::tr("Set share password");
    actionKeys[MenuAction::FormatDevice] = QObject::tr("Format");

    ///###: tag protocol.
    actionKeys[MenuAction::TagInfo] = QObject::tr("Tag information");
    actionKeys[MenuAction::TagFilesUseColor] = QString{"Add color tags"};
    actionKeys[MenuAction::DeleteTags] = QObject::tr("Delete");
    actionKeys[MenuAction::ChangeTagColor] = QString{"Change color of present tag"};
    actionKeys[MenuAction::RenameTag] = QObject::tr("Rename");
}

void DFileMenuData::initActions()
{
    QList<MenuAction> unCachedActions;
    unCachedActions << MenuAction::NewWindow;
    foreach (MenuAction key, actionKeys.keys()) {
        if (unCachedActions.contains(key)){
            continue;
        }

        ///###: MenuAction::TagFilesUseColor represents the button for tag files.
        ///###: MenuAction::ChangeTagColor represents that you change the color of a present tag.
        ///###: They are different event.
        if(key == MenuAction::TagFilesUseColor || key == MenuAction::ChangeTagColor){
            DTagActionWidget* tagWidget{ new DTagActionWidget };
            QWidgetAction* tagAction{ new QWidgetAction{ nullptr } };

            tagAction->setDefaultWidget(tagWidget);

            switch(key)
            {
            case MenuAction::TagFilesUseColor:
            {
                tagAction->setText("Add color tags");
                break;
            }
            case MenuAction::ChangeTagColor:
            {
                tagAction->setText("Change color of present tag");
                tagWidget->setExclusive(true);
                tagWidget->setToolTipVisible(false);
                break;
            }
            default:
                break;
            }

            tagAction->setData(key);
            actions.insert(key, tagAction);
            actionToMenuAction.insert(tagAction, key);
            continue;
        }

        QAction* action = new QAction(actionKeys.value(key), 0);
        action->setData(key);
        actions.insert(key, action);
        actionToMenuAction.insert(action, key);
    }
}

DFileMenu *DFileMenuManager::genereteMenuByKeys(const QVector<MenuAction> &keys,
                                               const QSet<MenuAction> &disableList,
                                               bool checkable,
                                               const QMap<MenuAction, QVector<MenuAction> > &subMenuList, bool isUseCachedAction, bool isRecursiveCall)
{
    static bool actions_initialized = false;

    if (!actions_initialized) {
        actions_initialized = true;
        DFileMenuData::initData();
        DFileMenuData::initActions();
    }

    if (!isUseCachedAction){
        foreach (MenuAction actionKey, keys) {
            QAction *action = DFileMenuData::actions.take(actionKey);

            if (action)
                DFileMenuData::actionToMenuAction.remove(action);
        }
    }

    DFileMenu* menu = new DFileMenu;

    if (!isRecursiveCall){
        connect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);
    }

    foreach (MenuAction key, keys) {
        if (!isAvailableAction(key))
            continue;

        if (key == MenuAction::Separator){
            menu->addSeparator();
        }else{
            QAction *action = DFileMenuData::actions.value(key);

            if(!action){
                action = new QAction(DFileMenuData::actionKeys.value(key), 0);
                action->setData(key);
                DFileMenuData::actions[key] = action;
                DFileMenuData::actionToMenuAction[action] = key;
            }

            action->setDisabled(disableList.contains(key));
            action->setProperty("_dfm_menu", QVariant::fromValue(menu));

            menu->addAction(action);

            if(!subMenuList.contains(key))
                continue;

            DFileMenu *subMenu = genereteMenuByKeys(subMenuList.value(key), disableList, checkable, QMap<MenuAction, QVector<MenuAction> >(), true, true);

            subMenu->QObject::setParent(menu);
            action->setMenu(subMenu);
        }
    }

    return menu;
}

QString DFileMenuManager::getActionString(MenuAction type)
{
    return DFileMenuData::actionKeys.value(type);
}

void DFileMenuManager::addActionWhitelist(MenuAction action)
{
    DFileMenuData::whitelist << action;
}

void DFileMenuManager::setActionWhitelist(const QSet<MenuAction> &actionList)
{
    DFileMenuData::whitelist = actionList;
}

QSet<MenuAction> DFileMenuManager::actionWhitelist()
{
    return DFileMenuData::whitelist;
}

void DFileMenuManager::addActionBlacklist(MenuAction action)
{
    DFileMenuData::blacklist << action;
}

void DFileMenuManager::setActionBlacklist(const QSet<MenuAction> &actionList)
{
    DFileMenuData::blacklist = actionList;
}

QSet<MenuAction> DFileMenuManager::actionBlacklist()
{
    return DFileMenuData::blacklist;
}

bool DFileMenuManager::isAvailableAction(MenuAction action)
{
    // init menu action black list
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(),
                       "dde-file-manager/" + qApp->applicationName());

    settings.setIniCodec("utf-8");
    settings.beginGroup("Menu Actions");
    const QMetaEnum &action_enum = QMetaEnum::fromType<MenuAction>();

    for (const QString &action_name : settings.value("disable").toStringList()) {
        bool ok = false;
        int key = action_enum.keyToValue(action_name.toUtf8(), &ok);

        if (ok && key == action)
            return false;
    }

    settings.endGroup();

    if (DFileMenuData::whitelist.isEmpty())
        return !DFileMenuData::blacklist.contains(action);

    return DFileMenuData::whitelist.contains(action) && !DFileMenuData::blacklist.contains(action);
}

void DFileMenuManager::setActionString(MenuAction type, QString actionString)
{
    DFileMenuData::actionKeys.insert(type, actionString);

    QAction* action = new QAction(actionString, 0);
    action->setData(type);
    DFileMenuData::actions.insert(type, action);
    DFileMenuData::actionToMenuAction[action] = type;
}

void DFileMenuManager::setActionID(MenuAction type, QString id)
{
    DFileMenuData::actionIDs.insert(type, id);
}

MenuAction DFileMenuManager::registerMenuActionType(QAction *action)
{
    Q_ASSERT(action);

    MenuAction type = DFileMenuData::actionToMenuAction.value(action, MenuAction::Unknow);

    if (type >= MenuAction::UserMenuAction)
        return type;

    type = DFileMenuData::takeAvailableUserActionType();
    DFileMenuData::actions[type] = action;
    DFileMenuData::actionToMenuAction[action] = type;

    QObject::connect(action, &QAction::destroyed, action, [type] {
        DFileMenuData::recycleUserActionType(type);
    });

    return type;
}

void DFileMenuManager::actionTriggered(QAction *action)
{
    qDebug() << action << action->data().isValid();
    DFileMenu *menu = qobject_cast<DFileMenu *>(sender());
    if (!(menu->property("ToolBarSettingsMenu").isValid() && menu->property("ToolBarSettingsMenu").toBool()))
        disconnect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);
    if (action->data().isValid()) {
        bool flag = false;
        int _type = action->data().toInt(&flag);
        MenuAction type;
        if (flag){
            type = (MenuAction)_type;
        }else{
            qDebug() << action->data().toString();;
            return;
        }

        if (type >= MenuAction::UserMenuAction)
            return;

        if (menu->ignoreMenuActions().contains(type)){
            return;
        }

        QAction* typeAction = DFileMenuData::actions.value(type);
        qDebug() << typeAction << action;
        if (typeAction){
            qDebug() << typeAction->text() << action->text();
            if (typeAction->text() == action->text()) {
                const QSharedPointer<DFMMenuActionEvent> &event = menu->makeEvent(type);
                DFMEventDispatcher::instance()->processEvent(event);
            }
        }

#ifdef SW_LABEL
        if (DFileMenuData::actionIDs.contains(type)){
            const QSharedPointer<DFMMenuActionEvent> &menuActionEvent = menu->makeEvent(type);
            DFMEvent event;
            event.setWindowId(menuActionEvent->windowId());
            event.setData(menuActionEvent->fileUrl());
            QMetaObject::invokeMethod(appController,
                                      "actionByIds",
                                      Qt::DirectConnection,
                                      Q_ARG(DFMEvent, event), Q_ARG(QString, DFileMenuData::actionIDs.value(type)));
        }
 #endif
    }
}
