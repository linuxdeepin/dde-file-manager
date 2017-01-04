#include "dfilemenumanager.h"
#include "dfmglobal.h"
#include "app/define.h"
#include "dfmevent.h"
#include "dfilemenu.h"
#include "dfileservices.h"
#include "controllers/appcontroller.h"
#include "controllers/trashmanager.h"

#include "models/computerdesktopfileinfo.h"
#include "models/trashdesktopfileinfo.h"

#include "widgets/singleton.h"
#include "views/windowmanager.h"
#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "controllers/pathmanager.h"
#include "plugins/pluginmanager.h"
#include "dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h"
#include "dfmstandardpaths.h"
#include <QMetaObject>
#include <QMetaEnum>
#include <QMenu>
#include <QSettings>
#include <QTextCodec>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace DFileMenuData {
static QMap<MenuAction, QString> actionKeys;
static QMap<MenuAction, QAction*> actions;
static QVector<MenuAction> sortActionTypes;
static QSet<MenuAction> whitelist;
static QSet<MenuAction> blacklist;

void initData();
void initActions();
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
               << MenuAction::Rename
               << MenuAction::Remove
               << MenuAction::Property;

    const DAbstractFileInfoPointer& info = fileService->createFileInfo(url);
    info->refresh();
    if (!info->exists()){
        disableList << MenuAction::OpenInNewWindow
                    << MenuAction::OpenInNewTab
                    << MenuAction::Rename
                    << MenuAction::Property;
    }else{
        if (!info->canRename()){
            disableList << MenuAction::Rename;
        }
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
               << MenuAction::Separator
               << MenuAction::SetUserSharePassword;
//               << MenuAction::Settings
    #ifndef ARCH_MIPSEL
        actionKeys  << MenuAction::Help;
    #endif
    actionKeys  << MenuAction::About
                << MenuAction::Exit;

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

DFileMenu *DFileMenuManager::createNormalMenu(const DUrl &currentUrl, const DUrlList &urlList, QSet<MenuAction> disableList, QSet<MenuAction> unusedList, int windowId)
{
    DAbstractFileInfoPointer info = fileService->createFileInfo(currentUrl);
    DFileMenu *menu = NULL;
    if (urlList.length() == 1) {
        if(currentUrl == ComputerDesktopFileInfo::computerDesktopFileUrl())
            unusedList << MenuAction::Property;
        QVector<MenuAction> actions = info->menuActionList(DAbstractFileInfo::SingleFile);

#ifdef DDE_COMPUTER_TRASH
        if(currentUrl == TrashDesktopFileInfo::trashDesktopFileUrl()){
            DAbstractFileInfoPointer trashFileInfo = fileService->createFileInfo(DUrl::fromTrashFile("/"));
            actions << MenuAction::ClearTrash;
            if(trashFileInfo->filesCount() <= 0)
                disableList += MenuAction::ClearTrash;
        }
#endif

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
            QMimeType mimeType = info->mimeType();
            QStringList recommendApps = mimeAppsManager->MimeApps.value(mimeType.name());

            foreach (QString name, mimeType.aliases()) {
                QStringList apps = mimeAppsManager->MimeApps.value(name);
                foreach (QString app, apps) {
                    if (!recommendApps.contains(app)){
                        recommendApps.append(app);
                    }
                }
            }

            foreach (QString app, recommendApps) {
                QAction* action = new QAction(mimeAppsManager->DesktopObjs.value(app).getLocalName(), 0);
                action->setIcon(FileUtils::searchAppIcon(mimeAppsManager->DesktopObjs.value(app)));
                action->setProperty("app", app);
                action->setProperty("url", info->fileUrl());
                openWithMenu->addAction(action);
                connect(action, &QAction::triggered, appController, &AppController::actionOpenFileByApp);
            }

            QAction* action = new QAction(fileMenuManger->getActionString(MenuAction::OpenWithCustom), 0);
            action->setData((int)MenuAction::OpenWithCustom);
            openWithMenu->addAction(action);

        }
    } else {
        bool isSystemPathIncluded = false;
        bool isAllCompressedFiles = true;

        foreach (DUrl url, urlList) {
            const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);

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

        const QMap<MenuAction, QVector<MenuAction> > subActions;
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

    if(currentUrl == ComputerDesktopFileInfo::computerDesktopFileUrl() ||
            currentUrl == TrashDesktopFileInfo::trashDesktopFileUrl())
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
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(fileUrl);

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
    actionKeys[MenuAction::OpenWithCustom] = QObject::tr("Others");
    actionKeys[MenuAction::OpenFileLocation] = QObject::tr("Open file loaction");
    actionKeys[MenuAction::Compress] = QObject::tr("Compress");
    actionKeys[MenuAction::Decompress] = QObject::tr("Extract");
    actionKeys[MenuAction::DecompressHere] = QObject::tr("Extract here");
    actionKeys[MenuAction::Cut] = QObject::tr("Cut");
    actionKeys[MenuAction::Copy] = QObject::tr("Copy");
    actionKeys[MenuAction::Paste] = QObject::tr("Paste");
    actionKeys[MenuAction::Rename] = QObject::tr("Rename");
    actionKeys[MenuAction::Remove] = QObject::tr("Remove");
    actionKeys[MenuAction::CreateSymlink] = QObject::tr("Create link");
    actionKeys[MenuAction::SendToDesktop] = QObject::tr("Send to desktop");
    actionKeys[MenuAction::AddToBookMark] = QObject::tr("Add to bookmark");
    actionKeys[MenuAction::Delete] = QObject::tr("Throw to Trash");
    actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Permanently delete");
    actionKeys[MenuAction::Property] = QObject::tr("Property");

    actionKeys[MenuAction::NewFolder] = QObject::tr("New folder");
    actionKeys[MenuAction::NewWindow] = QObject::tr("New window");
    actionKeys[MenuAction::SelectAll] = QObject::tr("Select all");
    actionKeys[MenuAction::ClearRecent] = QObject::tr("Clear recent history");
    actionKeys[MenuAction::ClearTrash] = QObject::tr("Empty Trash");
    actionKeys[MenuAction::DisplayAs] = QObject::tr("Display as");
    actionKeys[MenuAction::SortBy] = QObject::tr("Sort by");
    actionKeys[MenuAction::NewDocument] = QObject::tr("New document");
    actionKeys[MenuAction::NewWord] = QObject::tr("Word");
    actionKeys[MenuAction::NewExcel] = QObject::tr("Excel");
    actionKeys[MenuAction::NewPowerpoint] = QObject::tr("PowerPoint");
    actionKeys[MenuAction::NewText] = QObject::tr("Text");
    actionKeys[MenuAction::OpenInTerminal] = QObject::tr("Open in terminal");
    actionKeys[MenuAction::Restore] = QObject::tr("Restore");
    actionKeys[MenuAction::RestoreAll] = QObject::tr("Restore all");
    actionKeys[MenuAction::Mount] = QObject::tr("Mount");
    actionKeys[MenuAction::Unmount]= QObject::tr("Unmount");
    actionKeys[MenuAction::Eject]= QObject::tr("Eject");
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
}

void DFileMenuData::initActions()
{
    QList<MenuAction> unCachedActions;
    unCachedActions << MenuAction::NewWindow;
    foreach (MenuAction key, actionKeys.keys()) {
        if (unCachedActions.contains(key)){
            continue;
        }
        QAction* action = new QAction(actionKeys.value(key), 0);
        action->setData(key);
        actions.insert(key, action);
    }
}

DFileMenu *DFileMenuManager::genereteMenuByKeys(const QVector<MenuAction> &keys,
                                               const QSet<MenuAction> &disableList,
                                               bool checkable,
                                               const QMap<MenuAction, QVector<MenuAction> > &subMenuList, bool isUseCachedAction)
{
    if(DFileMenuData::actions.isEmpty()) {
        DFileMenuData::initData();
        DFileMenuData::initActions();
    }

    if (!isUseCachedAction){
        foreach (MenuAction actionKey, keys) {
            DFileMenuData::actions.remove(actionKey);
        }
    }

    DFileMenu* menu = new DFileMenu;

    connect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);

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
            }

            action->setDisabled(disableList.contains(key));

            menu->addAction(action);

            if(!subMenuList.contains(key))
                continue;

            DFileMenu *subMenu = genereteMenuByKeys(subMenuList.value(key), disableList, checkable);

//            subMenu->setParent(action);
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
    if (DFileMenuData::whitelist.isEmpty())
        return !DFileMenuData::blacklist.contains(action);

    return DFileMenuData::whitelist.contains(action) && !DFileMenuData::blacklist.contains(action);
}

void DFileMenuManager::actionTriggered(QAction *action)
{
    DFileMenu *menu = qobject_cast<DFileMenu *>(sender());
    DFMEvent event = menu->event();
    event << DFMEvent::Menu;
    if (action->data().isValid()){
        bool flag = false;
        int _type = action->data().toInt(&flag);
        MenuAction type;
        if (flag){
            type = (MenuAction)_type;
        }else{
            qDebug() << action->data().toString();;
            return;
        }

        QMetaEnum metaEnum = QMetaEnum::fromType<MenuAction>();
        QString key = metaEnum.valueToKey(type);
        QString methodKey = QString("action%1").arg(key);
        QString methodSignature = QString("action%1(" QT_STRINGIFY(DFMEvent) ")").arg(key);

        const QMetaObject* metaObject = appController->metaObject();
//        QStringList methods;
//        for(int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i){
//            methods << QString::fromLatin1(metaObject->method(i).methodSignature());
//        }
//        qDebug() << methods;
//        qDebug() << methodKey << methodName;
        if (metaObject->indexOfSlot(methodSignature.toLocal8Bit().constData()) != -1){
            QMetaObject::invokeMethod(appController,
                                      methodKey.toLocal8Bit().constData(),
                                      Qt::DirectConnection,
                                      Q_ARG(DFMEvent, event));
        }else{
            qWarning() << "Appcontroller has no method:" << methodSignature;
        }
    }
}
