#include "filemenumanager.h"
#include "dfilemenu.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"

#include "../controllers/fileservices.h"

#include "../dialogs/propertydialog.h"

#include <QScreen>

QMap<MenuAction, QString> FileMenuManager::m_actionKeys;
QMap<MenuAction, DAction*> FileMenuManager::m_actions;

DFileMenu *FileMenuManager::createRecentLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << MenuAction::OpenInNewWindow << MenuAction::ClearRecent;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDefaultBookMarkMenu()
{
    QVector<MenuAction> actionKeys;
    QVector<MenuAction> disableList;

    actionKeys.reserve(7);

    actionKeys << MenuAction::Open
               << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Remove
               << MenuAction::Rename
               << MenuAction::Separator
               << MenuAction::Property;
    disableList << MenuAction::Remove
                << MenuAction::Rename;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createCustomBookMarkMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(10);

    actionKeys << MenuAction::Open
               << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Copy
               << MenuAction::Cut
               << MenuAction::Rename
               << MenuAction::Separator
               << MenuAction::Remove
               << MenuAction::Separator
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Open << MenuAction::OpenInNewWindow
               << MenuAction::Separator << MenuAction::ClearTrash;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Open
               << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskViewMenu(const QVector<MenuAction> &disableList)
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

DFileMenu *FileMenuManager::createToolBarSettingsMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::NewWindow << MenuAction::Separator
               << MenuAction::Help
               << MenuAction::About
               << MenuAction::Exit;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSortMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::Name << MenuAction::Size
               << MenuAction::Type
               << MenuAction::CreatedDate
               << MenuAction::LastModifiedDate;

    return genereteMenuByKeys(actionKeys, disableList, true);
}

DFileMenu *FileMenuManager::createListViewHeaderMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Size
               << MenuAction::Type
               << MenuAction::CreatedDate
               << MenuAction::LastModifiedDate;

    return genereteMenuByKeys(actionKeys, disableList, true);
}

QVector<MenuAction> FileMenuManager::getDisableActionList(const DUrl &fileUrl)
{
    AbstractFileInfo *fileInfo = fileService->createFileInfo(fileUrl);
    QVector<MenuAction> disableList;

    if(!fileInfo->isCanRename())
        disableList << MenuAction::Rename;

    if(!fileInfo->isReadable())
        disableList << MenuAction::Open << MenuAction::OpenWith
                    << MenuAction::OpenInNewWindow << MenuAction::Copy;

    AbstractFileInfo *parentInfo = fileService->createFileInfo(fileInfo->parentUrl());

    if(!fileInfo->isWritable())
        disableList << MenuAction::Paste << MenuAction::NewDocument
                    << MenuAction::NewFile << MenuAction::NewFolder;

    if(!fileInfo->isWritable() || (parentInfo->exists() && !parentInfo->isWritable()))
        disableList << MenuAction::Cut << MenuAction::Remove
                    << MenuAction::Delete << MenuAction::CompleteDeletion;

    delete fileInfo;
    delete parentInfo;

    return disableList;
}

FileMenuManager::FileMenuManager()
{
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
}

void FileMenuManager::initData()
{
    m_actionKeys[MenuAction::Open] = QObject::tr("Open");
    m_actionKeys[MenuAction::OpenInNewWindow] = QObject::tr("Open in new window");
    m_actionKeys[MenuAction::OpenWith] = QObject::tr("Open with");
    m_actionKeys[MenuAction::OpenFileLocation] = QObject::tr("Open file loaction");
    m_actionKeys[MenuAction::Compress] = QObject::tr("Compress");
    m_actionKeys[MenuAction::Decompress] = QObject::tr("Decompress");
    m_actionKeys[MenuAction::Cut] = QObject::tr("Cut");
    m_actionKeys[MenuAction::Copy] = QObject::tr("Copy");
    m_actionKeys[MenuAction::Paste] = QObject::tr("Paste");
    m_actionKeys[MenuAction::Rename] = QObject::tr("Rename");
    m_actionKeys[MenuAction::Remove] = QObject::tr("Remove");
    m_actionKeys[MenuAction::Delete] = QObject::tr("Delete");
    m_actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Complete deletion");
    m_actionKeys[MenuAction::Property] = QObject::tr("Property");

    m_actionKeys[MenuAction::NewFolder] = QObject::tr("New Folder");
    m_actionKeys[MenuAction::NewFile] = QObject::tr("New File");
    m_actionKeys[MenuAction::NewWindow] = QObject::tr("New Window");
    m_actionKeys[MenuAction::SelectAll] = QObject::tr("Select All");
    m_actionKeys[MenuAction::ClearRecent] = QObject::tr("Clear Recent");
    m_actionKeys[MenuAction::ClearTrash] = QObject::tr("Clear Trash");
    m_actionKeys[MenuAction::DisplayAs] = QObject::tr("Display As");
    m_actionKeys[MenuAction::SortBy] = QObject::tr("Sort By");
    m_actionKeys[MenuAction::NewDocument] = QObject::tr("New Document");
    m_actionKeys[MenuAction::Restore] = QObject::tr("Restore");
    m_actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Complete Deletion");
    m_actionKeys[MenuAction::Mount] = QObject::tr("Mount");
    m_actionKeys[MenuAction::Unmount]= QObject::tr("Unmount");
    m_actionKeys[MenuAction::Name] = QObject::tr("Name");
    m_actionKeys[MenuAction::Size] = QObject::tr("Size");
    m_actionKeys[MenuAction::Type] = QObject::tr("Type");
    m_actionKeys[MenuAction::CreatedDate] = QObject::tr("Created Date");
    m_actionKeys[MenuAction::LastModifiedDate] = QObject::tr("Last Modified Date");
    m_actionKeys[MenuAction::Help] = QObject::tr("Help");
    m_actionKeys[MenuAction::About] = QObject::tr("About");
    m_actionKeys[MenuAction::Exit] = QObject::tr("Exit");
    m_actionKeys[MenuAction::IconView] = QObject::tr("Icon View");
    m_actionKeys[MenuAction::ListView] = QObject::tr("List View");
}

void FileMenuManager::initActions()
{
    foreach (MenuAction key, m_actionKeys.keys()) {
        DAction* action = new DAction(m_actionKeys.value(key), 0);

        action->setData(key);

        m_actions.insert(key, action);
    }
}

DFileMenu *FileMenuManager::genereteMenuByKeys(const QVector<MenuAction> &keys,
                                               const QVector<MenuAction> &disableList,
                                               bool checkable,
                                               const QMap<MenuAction, QVector<MenuAction> > &subMenuList)
{
    if(m_actions.isEmpty()) {
        initData();
        initActions();
    }

    DFileMenu* menu = new DFileMenu;
    connect(menu, &DFileMenu::triggered, fileMenuManger, &FileMenuManager::actionTriggered);

    foreach (MenuAction key, keys) {
        if (key == MenuAction::Separator){
            menu->addSeparator();
        }else{
            DAction *action = m_actions.value(key);

            if(!action)
                continue;

            action->setCheckable(checkable);
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

void FileMenuManager::actionTriggered(DAction *action)
{
    DFileMenu *menu = qobject_cast<DFileMenu *>(sender());
    DUrlList urls = menu->getUrls();
    MenuAction type = (MenuAction)action->data().toInt();
    DUrl fileUrl;

    if(urls.size() > 0)
        fileUrl = urls.first();

    switch(type)
    {
    case MenuAction::Open: {
        FMEvent event;

        event = fileUrl;
        event = FMEvent::Menu;
        event = menu->getWindowId();
        qDebug() << event;
        fileService->openUrl(event);
        break;
    }
    case MenuAction::OpenInNewWindow:
        fileService->openNewWindow(fileUrl);
        break;
    case MenuAction::OpenWith:
        emit fileSignalManager->requestShowOpenWithDialog(fileUrl);
        break;
    case MenuAction::OpenFileLocation:
        fileService->openFileLocation(fileUrl);
        break;
    case MenuAction::Compress:break;
    case MenuAction::Decompress:break;
    case MenuAction::Cut:
        fileService->cutFiles(urls);
        break;
    case MenuAction::Copy:
        fileService->copyFiles(urls);
        break;
    case MenuAction::Paste: {
        FMEvent event;

        event = fileUrl;
        event = FMEvent::Menu;
        event = menu->getWindowId();

        fileService->pasteFile(event);
        break;
    }
    case MenuAction::Rename: {
        FMEvent event;

        event = fileUrl;
        event = FMEvent::Menu;
        event = menu->getWindowId();

        emit fileSignalManager->requestRename(event);
        break;
    }
    case MenuAction::Remove:
    {
        FMEvent event;

        event = fileUrl;
        event = FMEvent::Menu;
        event = menu->getWindowId();
        fileSignalManager->requestBookmarkRemove(event);
        break;
    }
    case MenuAction::Delete:
        fileService->moveToTrash(urls);
        break;
    case MenuAction::CompleteDeletion:
        fileService->deleteFiles(urls);
        break;
    case MenuAction::NewFolder:
        fileService->newFolder(fileUrl);
        break;
    case MenuAction::NewFile:
        fileService->newFile(fileUrl);
        break;
    case MenuAction::NewWindow:break;
    case MenuAction::SelectAll:
        fileSignalManager->requestViewSelectAll(menu->getWindowId());
        break;
    case MenuAction::ClearRecent:break;
    case MenuAction::ClearTrash:
        fileService->deleteFiles(DUrlList() << TRASHURL);
        break;
    case MenuAction::DisplayAs:break;
    case MenuAction::SortBy:break;
    case MenuAction::NewDocument:break;
    case MenuAction::Mount:
        deviceListener->mount(fileUrl.query());
        break;
    case MenuAction::Unmount:
        deviceListener->unmount(fileUrl.query());
        break;
    case MenuAction::Restore:
        fileSignalManager->requestRestoreTrashFile(fileUrl);
        break;
    case MenuAction::Name:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileDisplayNameRole);
        break;
    case MenuAction::Size:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileSizeRole);
        break;
    case MenuAction::Type:
        fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileMimeTypeRole);
        break;
    case MenuAction::CreatedDate:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileCreated);
        break;
    case MenuAction::LastModifiedDate:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileLastModified);
        break;
    case MenuAction::Property: {
        emit fileSignalManager->requestShowPropertyDialog(fileUrl);
        break;
    }
    case MenuAction::Help:break;
    case MenuAction::About:break;
    case MenuAction::Exit:break;
    case MenuAction::IconView:break;
    case MenuAction::ListView:break;
    default:
        qDebug() << "unknown action type";
        break;
    }
}
