#include "filemenumanager.h"
#include "dfilemenu.h"

QMap<FileMenuManager::MenuAction, QString> FileMenuManager::m_actionKeys;
QMap<FileMenuManager::MenuAction, DAction*> FileMenuManager::m_actions;

DFileMenu *FileMenuManager::createFileMenu()
{
    QList<MenuAction> actionKeys;

    actionKeys << Open << OpenInNewWindow
               << Separator
               << Compress << Cut << Copy
               << Separator
               << Rename << Delete
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys);
}

DFileMenu *FileMenuManager::createViewSpaceAreaMenu()
{
    QList<MenuAction> actionKeys;

    actionKeys << NewFolder << NewDoc
               << Separator
               << Paste
               << Separator
               << SelectAll
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys);
}

FileMenuManager::FileMenuManager()
{

}

void FileMenuManager::initData()
{
    m_actionKeys[Open] = QObject::tr("Open");
    m_actionKeys[OpenInNewWindow] = QObject::tr("Open in new window");
    m_actionKeys[OpenWith] = QObject::tr("Open with");
    m_actionKeys[Compress] = QObject::tr("Compress");
    m_actionKeys[Decompress] = QObject::tr("Decompress");
    m_actionKeys[Cut] = QObject::tr("Cut");
    m_actionKeys[Copy] = QObject::tr("Copy");
    m_actionKeys[Paste] = QObject::tr("Paste");
    m_actionKeys[Rename] = QObject::tr("Rename");
    m_actionKeys[Delete] = QObject::tr("Delete");
    m_actionKeys[Property] = QObject::tr("Property");

    m_actionKeys[NewFolder] = QObject::tr("New Folder");
    m_actionKeys[NewFile] = QObject::tr("New File");
    m_actionKeys[NewDoc] = QObject::tr("New Doc");
    m_actionKeys[SelectAll] = QObject::tr("Select all");
}

void FileMenuManager::initActions()
{
    foreach (MenuAction key, m_actionKeys.keys()) {
        DAction* action = new DAction(m_actionKeys.value(key), 0);

        action->setData(key);

        m_actions.insert(key, action);
    }
}

DFileMenu *FileMenuManager::genereteMenuByKeys(const QList<MenuAction> keys)
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
            if (m_actions.contains(key)){
                menu->addAction(m_actions.value(key));
            }
        }
    }

    return menu;
}
