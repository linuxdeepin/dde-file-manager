#include "dfilemenumanager.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filemanagerapp.h"
#include "dfilemenu.h"
#include "dfileservices.h"
#include "controllers/appcontroller.h"
#include "controllers/trashmanager.h"

#include "widgets/singleton.h"

#include <QMetaObject>
#include <QMetaEnum>
#include <QDebug>

namespace DFileMenuData {
static QMap<MenuAction, QString> actionKeys;
static QMap<MenuAction, DAction*> actions;
static QVector<MenuAction> sortActionTypes;

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

DFileMenu *DFileMenuManager::createCustomBookMarkMenu(const DUrl &url, QSet<MenuAction> disableList)
{

    QVector<MenuAction> actionKeys;

    actionKeys.reserve(10);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab
               << MenuAction::Rename
               << MenuAction::Remove
               << MenuAction::Property;

    if (!QDir(url.path()).exists()){
        disableList << MenuAction::OpenInNewWindow
                          << MenuAction::Rename
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

    actionKeys.reserve(5);

    actionKeys << MenuAction::NewWindow
               << MenuAction::Separator;
//               << MenuAction::Settings
    #ifndef ARCH_MIPSEL
        actionKeys  << MenuAction::Help;
    #endif
    actionKeys  << MenuAction::About
                << MenuAction::Exit;

    return genereteMenuByKeys(actionKeys, disableList);
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
        const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(fileUrl);

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
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
}

void DFileMenuData::initData()
{
    actionKeys[MenuAction::Open] = QObject::tr("Open");
    actionKeys[MenuAction::OpenInNewWindow] = QObject::tr("Open in new window");
    actionKeys[MenuAction::OpenInNewTab] = QObject::tr("Open in new tab");
    actionKeys[MenuAction::OpenDisk] = QObject::tr("Open");
    actionKeys[MenuAction::OpenDiskInNewWindow] = QObject::tr("Open in new window");
    actionKeys[MenuAction::OpenAsAdmain] = QObject::tr("Open in new window as admain");
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
    actionKeys[MenuAction::UnShare] = QObject::tr("Cancle sharing");
}

void DFileMenuData::initActions()
{
    foreach (MenuAction key, actionKeys.keys()) {
        DAction* action = new DAction(actionKeys.value(key), 0);
        action->setData(key);
        actions.insert(key, action);
    }
}

DFileMenu *DFileMenuManager::genereteMenuByKeys(const QVector<MenuAction> &keys,
                                               const QSet<MenuAction> &disableList,
                                               bool checkable,
                                               const QMap<MenuAction, QVector<MenuAction> > &subMenuList)
{
    if(DFileMenuData::actions.isEmpty()) {
        DFileMenuData::initData();
        DFileMenuData::initActions();
    }

    DFileMenu* menu = new DFileMenu;

    connect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);

    foreach (MenuAction key, keys) {
        if (key == MenuAction::Separator){
            menu->addSeparator();
        }else{
            DAction *action = DFileMenuData::actions.value(key);

            if(!action)
                continue;

            action->setCheckable(true);
            action->setDisabled(disableList.contains(key));

            menu->addAction(action);

            if(!subMenuList.contains(key))
                continue;

            DFileMenu *subMenu = genereteMenuByKeys(subMenuList.value(key), disableList, checkable);

            subMenu->setParent(action);
            action->setMenu(subMenu);
        }
    }

    return menu;
}

QString DFileMenuManager::getActionString(MenuAction type)
{
    if (DFileMenuData::actionKeys.contains(type)){
        return DFileMenuData::actionKeys.value(type);
    }
    return "";
}

void DFileMenuManager::actionTriggered(DAction *action)
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
        QString methodSignature = QString("action%1(FMEvent)").arg(key);

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
