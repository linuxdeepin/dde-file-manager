#include "filemenumanager.h"
#include "dfilemenu.h"

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
               << OpenParentFolder
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
    QVector<MenuAction> sortBySubActionKeys;

    sortBySubActionKeys << Name << Size << Type << CreatedDate << LastModifiedDate;

    subMenu[SortBy] = sortBySubActionKeys;

    return genereteMenuByKeys(actionKeys, disableList, false, subMenu);
}

DFileMenu *FileMenuManager::createDefaultBookMarkMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(7);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Remove
               << Rename
               << Separator
               << Property;

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

FileMenuManager::FileMenuManager()
{

}

void FileMenuManager::initData()
{
    m_actionKeys[Open] = QObject::tr("Open");
    m_actionKeys[OpenInNewWindow] = QObject::tr("Open in new window");
    m_actionKeys[OpenWith] = QObject::tr("Open with");
    m_actionKeys[OpenParentFolder] = QObject::tr("OpenParentFolder");
    m_actionKeys[Compress] = QObject::tr("Compress");
    m_actionKeys[Decompress] = QObject::tr("Decompress");
    m_actionKeys[Cut] = QObject::tr("Cut");
    m_actionKeys[Copy] = QObject::tr("Copy");
    m_actionKeys[Paste] = QObject::tr("Paste");
    m_actionKeys[Rename] = QObject::tr("Rename");
    m_actionKeys[Remove] = QObject::tr("Remove");
    m_actionKeys[Delete] = QObject::tr("Delete");
    m_actionKeys[Property] = QObject::tr("Property");

    m_actionKeys[NewFolder] = QObject::tr("New Folder");
    m_actionKeys[NewFile] = QObject::tr("New File");
    m_actionKeys[NewWindow] = QObject::tr("New Window");
    m_actionKeys[SelectAll] = QObject::tr("Select all");
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
