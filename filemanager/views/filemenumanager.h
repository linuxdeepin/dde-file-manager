#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QMap>

class DMenu;

class FileMenuManager
{
public:
    enum MenuAction {
        Open,
        OpenInNewWindow,
        OpenWith,
        Compress,
        Decompress,
        Cut,
        Copy,
        Paste,
        Rename,
        Delete,
        Property,
        NewFolder,
        NewFile,
        NewDoc,
        SelectAll,
        Separator
    };

    static DMenu *createFileMenu();
    static DMenu *createViewSpaceAreaMenu();

private:
    FileMenuManager();

    static QMap<MenuAction, QString> m_actionKeys;
    static QMap<MenuAction, QAction*> m_actions;

    static void initData();
    static void initActions();
    static DMenu *genereteMenuByKeys(const QList<MenuAction> keys);
};

#endif // FILEMENUMANAGER_H
