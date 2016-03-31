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
        OpenParentFolder,
        Compress,
        Decompress,
        Cut,
        Copy,
        Paste,
        Rename,
        Remove,
        Delete,
        Property,
        NewFolder,
        NewFile,
        NewWindow,
        SelectAll,
        Separator,
        ClearRecent,
        ClearTrash,
        DisplayAs, /// sub menu
        SortBy, /// sub menu
        NewDocument, /// sub menu
        Restore,
        CompleteDeletion,
        Mount,
        Unmount,
        Name,
        Size,
        Type,
        CreatedDate,
        LastModifiedDate,
        Help,
        About,
        Exit
    };

    static DFileMenu *createFileMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createFolderMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createViewSpaceAreaMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createRecentLeftBarMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createRecentFileMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createRecentViewSpaceAreaMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createDefaultBookMarkMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createCustomBookMarkMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createTrashLeftBarMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createTrashFileMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createTrashFolderMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createTrashViewSpaceAreaMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createDiskLeftBarMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createDiskViewMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createToolBarSettingsMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createToolBarSortMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createListViewHeaderMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());

private:
    FileMenuManager();

    static QMap<MenuAction, QString> m_actionKeys;
    static QMap<MenuAction, DAction*> m_actions;

    static void initData();
    static void initActions();
    static DFileMenu *genereteMenuByKeys(const QVector<MenuAction> &keys,
                                         const QVector<MenuAction> &disableList,
                                         bool checkable = false,
                                         const QMap<MenuAction, QVector<MenuAction> > &subMenuList = QMap<MenuAction, QVector<MenuAction> >());
};

#endif // FILEMENUMANAGER_H
