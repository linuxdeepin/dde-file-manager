#include "filemenumanager.h"
#include "dmenu.h"

QMap<FileMenuManager::MenuAction, QString> FileMenuManager::m_actionKeys;
QMap<FileMenuManager::MenuAction, QAction*> FileMenuManager::m_actions;

DMenu *FileMenuManager::createFileMenu()
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

DMenu *FileMenuManager::createViewSpaceAreaMenu()
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
        QAction* action = new QAction(m_actionKeys.value(key), 0);

        action->setData(key);

        m_actions.insert(key, action);
    }
}

DMenu *FileMenuManager::genereteMenuByKeys(const QList<MenuAction> keys)
{
    if(m_actions.isEmpty()) {
        initData();
        initActions();
    }

    DMenu* menu = new DMenu;

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
