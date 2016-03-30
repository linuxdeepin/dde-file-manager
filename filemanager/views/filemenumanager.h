#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include <QObject>
#include <QMap>

#include <DAction>

DWIDGET_USE_NAMESPACE

class DFileMenu;

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

    static DFileMenu *createFileMenu();
    static DFileMenu *createViewSpaceAreaMenu();

private:
    FileMenuManager();

    static QMap<MenuAction, QString> m_actionKeys;
    static QMap<MenuAction, DAction*> m_actions;

    static void initData();
    static void initActions();
    static DFileMenu *genereteMenuByKeys(const QList<MenuAction> keys);
};

#endif // FILEMENUMANAGER_H
