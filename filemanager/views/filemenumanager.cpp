#include "filemenumanager.h"
#include "dfilemenu.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"

#include "../controllers/fileservices.h"

#include "../dialogs/propertydialog.h"

#include <QScreen>

QMap<FileMenuManager::MenuAction, QString> FileMenuManager::m_actionKeys;
QMap<FileMenuManager::MenuAction, DAction*> FileMenuManager::m_actions;

DFileMenu *FileMenuManager::createFileMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(11);

    actionKeys << Open << OpenWith
               << Separator
               << Compress << Separator
               << Copy << Cut
               << Rename << Delete
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createFolderMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(11);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Compress << Separator
               << Copy << Cut
               << Rename << Delete
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createViewSpaceAreaMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(9);

    actionKeys << OpenInNewWindow
               << Separator
               << NewFolder << NewDocument
               << Separator
               << Paste
               << SelectAll
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createRecentLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << OpenInNewWindow << ClearRecent;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createRecentFileMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(12);

    actionKeys << Open << OpenWith
               << OpenFileLocation
               << Separator
               << Compress << Separator
               << Copy << Cut
               << Rename << Delete
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createRecentViewSpaceAreaMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << ClearRecent
               << Separator
               << DisplayAs
               << SortBy
               << Separator
               << Property;

    QMap<MenuAction, QVector<MenuAction> > subMenu;
    QVector<MenuAction> subActionKeys;

    subActionKeys << IconView << ListView;

    subMenu[DisplayAs] = subActionKeys;
    subActionKeys.clear();

    subActionKeys << Name << Size << Type << CreatedDate << LastModifiedDate;

    subMenu[SortBy] = subActionKeys;

    return genereteMenuByKeys(actionKeys, disableList, false, subMenu);
}

DFileMenu *FileMenuManager::createDefaultBookMarkMenu()
{
    QVector<MenuAction> actionKeys;
    QVector<MenuAction> disableList;

    actionKeys.reserve(7);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Remove
               << Rename
               << Separator
               << Property;
    disableList << Remove
                << Rename;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createCustomBookMarkMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(10);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Copy
               << Cut
               << Rename
               << Separator
               << Remove
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << Open << OpenInNewWindow
               << Separator << ClearTrash;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashFileMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(12);

    actionKeys << Open << OpenWith
               << Separator
               << Compress << Separator
               << Copy << Cut << Separator
               << Restore << CompleteDeletion
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashFolderMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(12);

    actionKeys << Open << OpenInNewWindow
               << Separator
               << Compress << Separator
               << Copy << Cut << Separator
               << Restore << CompleteDeletion
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashViewSpaceAreaMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(7);

    actionKeys << OpenInNewWindow
               << Separator
               << Paste
               << SelectAll
               << ClearTrash
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskLeftBarMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskViewMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << Open << OpenInNewWindow
               << Separator
               << Mount << Unmount
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSettingsMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << NewWindow << Separator
               << Help
               << About
               << Exit;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSortMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << Name << Size
               << Type
               << CreatedDate
               << LastModifiedDate;

    return genereteMenuByKeys(actionKeys, disableList, true);
}

DFileMenu *FileMenuManager::createListViewHeaderMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << Size
               << Type
               << CreatedDate
               << LastModifiedDate;

    return genereteMenuByKeys(actionKeys, disableList, true);
}

QVector<FileMenuManager::MenuAction> FileMenuManager::getDisableActionList(const QString &fileUrl)
{
    AbstractFileInfo *fileInfo = fileService->createFileInfo(fileUrl);
    QVector<FileMenuManager::MenuAction> disableList;

    if(!fileInfo->isCanRename())
        disableList << FileMenuManager::Rename;

    if(!fileInfo->isReadable())
        disableList << FileMenuManager::Open << FileMenuManager::OpenWith
                    << FileMenuManager::OpenInNewWindow << FileMenuManager::Copy;

    AbstractFileInfo *parentInfo = fileService->createFileInfo(fileInfo->scheme() + "://"
                                                               + fileInfo->absolutePath());

    if(!fileInfo->isWritable())
        disableList << FileMenuManager::Paste << FileMenuManager::NewDocument
                    << FileMenuManager::NewFile << FileMenuManager::NewFolder;

    if(!fileInfo->isWritable() || (parentInfo->exists() && !parentInfo->isWritable()))
        disableList << FileMenuManager::Cut << FileMenuManager::Remove
                    << FileMenuManager::Delete << FileMenuManager::CompleteDeletion;

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
    m_actionKeys[Open] = QObject::tr("Open");
    m_actionKeys[OpenInNewWindow] = QObject::tr("Open in new window");
    m_actionKeys[OpenWith] = QObject::tr("Open with");
    m_actionKeys[OpenFileLocation] = QObject::tr("Open file loaction");
    m_actionKeys[Compress] = QObject::tr("Compress");
    m_actionKeys[Decompress] = QObject::tr("Decompress");
    m_actionKeys[Cut] = QObject::tr("Cut");
    m_actionKeys[Copy] = QObject::tr("Copy");
    m_actionKeys[Paste] = QObject::tr("Paste");
    m_actionKeys[Rename] = QObject::tr("Rename");
    m_actionKeys[Remove] = QObject::tr("Remove");
    m_actionKeys[Delete] = QObject::tr("Delete");
    m_actionKeys[CompleteDeletion] = QObject::tr("Complete deletion");
    m_actionKeys[Property] = QObject::tr("Property");

    m_actionKeys[NewFolder] = QObject::tr("New Folder");
    m_actionKeys[NewFile] = QObject::tr("New File");
    m_actionKeys[NewWindow] = QObject::tr("New Window");
    m_actionKeys[SelectAll] = QObject::tr("Select All");
    m_actionKeys[ClearRecent] = QObject::tr("Clear Recent");
    m_actionKeys[ClearTrash] = QObject::tr("Clear Trash");
    m_actionKeys[DisplayAs] = QObject::tr("Display As");
    m_actionKeys[SortBy] = QObject::tr("Sort By");
    m_actionKeys[NewDocument] = QObject::tr("New Document");
    m_actionKeys[Restore] = QObject::tr("Restore");
    m_actionKeys[CompleteDeletion] = QObject::tr("Complete Deletion");
    m_actionKeys[Mount] = QObject::tr("Mount");
    m_actionKeys[Unmount]= QObject::tr("Unmount");
    m_actionKeys[Name] = QObject::tr("Name");
    m_actionKeys[Size] = QObject::tr("Size");
    m_actionKeys[Type] = QObject::tr("Type");
    m_actionKeys[CreatedDate] = QObject::tr("Created Date");
    m_actionKeys[LastModifiedDate] = QObject::tr("Last Modified Date");
    m_actionKeys[Help] = QObject::tr("Help");
    m_actionKeys[About] = QObject::tr("About");
    m_actionKeys[Exit] = QObject::tr("Exit");
    m_actionKeys[IconView] = QObject::tr("Icon View");
    m_actionKeys[ListView] = QObject::tr("List View");
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
        if (key == Separator){
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
    QList<QString> urls = menu->getUrls();
    MenuAction type = (MenuAction)action->data().toInt();
    QString fileUrl;

    if(urls.size() > 0)
        fileUrl = urls.first();

    switch(type)
    {
    case Open: {
        FMEvent event;

        event = fileUrl;
        event = FMEvent::Menu;
        event = menu->getWindowId();
        qDebug() << event;
        fileService->openUrl(event);
        break;
    }
    case OpenInNewWindow:
        fileService->openNewWindow(fileUrl);
        break;
    case OpenWith:break;
    case OpenFileLocation:
        fileService->openFileLocation(fileUrl);
        break;
    case Compress:break;
    case Decompress:break;
    case Cut:
        fileService->cutFiles(urls);
        break;
    case Copy:
        fileService->copyFiles(urls);
        break;
    case Paste: {
        FMEvent event;

        event = fileUrl;
        event = FMEvent::Menu;
        event = menu->getWindowId();

        fileService->pasteFile(event);
        break;
    }
    case Rename: {
        FMEvent event;

        event = fileUrl;
        event = FMEvent::Menu;
        event = menu->getWindowId();

        emit fileSignalManager->requestRename(event);
        break;
    }
    case Remove:
        fileSignalManager->requestBookmarkRemove(fileUrl);
        break;
    case Delete:
        fileService->moveToTrash(urls);
        break;
    case CompleteDeletion:
        fileService->deleteFiles(urls);
        break;
    case NewFolder:
        fileService->newFolder(fileUrl);
        break;
    case NewFile:
        fileService->newFile(fileUrl);
        break;
    case NewWindow:break;
    case SelectAll:
        fileSignalManager->requestViewSelectAll(menu->getWindowId());
        break;
    case ClearRecent:break;
    case ClearTrash:break;
    case DisplayAs:break;
    case SortBy:break;
    case NewDocument:break;
    case Restore:break;
    case Mount:break;
    case Unmount:break;
    case Name:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileNameRole);
        break;
    case Size:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileSizeRole);
        break;
    case Type:
        fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileMimeTypeRole);
        break;
    case CreatedDate:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileCreated);
        break;
    case LastModifiedDate:
        emit fileSignalManager->requestViewSort(menu->getWindowId(), Global::FileLastModified);
        break;
    case Property: {
        PropertyDialog *dialog = new PropertyDialog(menu->fileInfo());

        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowFlags(dialog->windowFlags()
                               &~ Qt::WindowMaximizeButtonHint
                               &~ Qt::WindowMinimizeButtonHint
                               &~ Qt::WindowSystemMenuHint);
        dialog->setTitle("");
        dialog->setFixedSize(QSize(320, 480));

        QRect dialog_geometry = dialog->geometry();

        dialog_geometry.moveCenter(qApp->primaryScreen()->geometry().center());
        dialog->move(dialog_geometry.topLeft());
        dialog->show();

        break;
    }
    case Help:break;
    case About:break;
    case Exit:break;
    case IconView:break;
    case ListView:break;
    default:
        qDebug() << "unknown action type";
        break;
    }
}
