#include "filemenumanager.h"
#include <QDebug>
#include "dmenu.h"

FileMenuManager::FileMenuManager(QObject *parent) : QObject(parent)
{
    initData();
    initActions();
}

FileMenuManager::~FileMenuManager()
{

}

void FileMenuManager::initData()
{
    m_actionKeys["open"] = tr("Open");
    m_actionKeys["openInNewWindow"] = tr("Open in new window");
    m_actionKeys["openWidth"] = tr("Open with");
    m_actionKeys["compress"] = tr("Compress");
    m_actionKeys["decompress"] = tr("Decompress");
    m_actionKeys["cut"] = tr("Cut");
    m_actionKeys["copy"] = tr("Copy");
    m_actionKeys["paste"] = tr("Paste");
    m_actionKeys["rename"] = tr("Rename");
    m_actionKeys["delete"] = tr("Delete");
    m_actionKeys["property"] = tr("Property");

    m_actionKeys["newFolder"] = tr("New Folder");
    m_actionKeys["newFile"] = tr("New File");
    m_actionKeys["newDoc"] = tr("New Doc");
    m_actionKeys["selectAll"] = tr("Select all");
}

void FileMenuManager::initActions()
{
    foreach (QString key, m_actionKeys.keys()) {
        QAction* action = new QAction(this);
        action->setText(m_actionKeys.value(key));
        action->setData(key);
        m_actions.insert(key, action);
    }
}

QMenu *FileMenuManager::genereteMenuByFileType(QString type)
{
    QStringList actionKeys;
    if (type == "File"){
        actionKeys << "open" << "openInNewWindow"
                   << "separator"
                   << "compress" << "cut" << "copy"
                   << "separator"
                   << "rename" << "delete"
                   << "separator"
                   << "property";
    }else if (type == "DesktopFile"){

    }else if (type == "CompressFile"){

    }else if (type == "Folder"){

    }else if (type == "Space"){
        actionKeys << "newFolder" << "newDoc"
                   << "separator"
                   << "paste"
                   << "separator"
                   << "selectAll"
                   << "separator"
                   << "property";
    }
    else{
        actionKeys << "open" << "openInNewWindow"
                   << "separator"
                   << "compress" << "cut" << "copy"
                   << "separator"
                   << "rename" << "delete"
                   << "separator"
                   << "property";
    }
    return genereteMenuByKeys(actionKeys);
}

QMenu *FileMenuManager::genereteMenuByKeys(const QStringList keys)
{
    QMenu* menu = new QMenu;
    menu->setObjectName("Menu");
    foreach (QString key, keys) {
        if (key == "separator"){
            menu->addSeparator();
        }else{
            if (m_actions.contains(key)){
                menu->addAction(m_actions.value(key));
            }
        }
    }
    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(handleAction(QAction*)));
    return menu;
}

void FileMenuManager::handleAction(QAction *action)
{
    qDebug() << sender()->property("url") << action->data();
}
