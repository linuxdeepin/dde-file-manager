#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include <QObject>
#include <QMap>

#include <DAction>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
DWIDGET_USE_NAMESPACE

class DFileMenu;
class DUrl;

class FileMenuManager : public QObject
{
    Q_OBJECT
public:
    enum MenuAction {
        Open,
        OpenInNewWindow,
        OpenWith,
        OpenFileLocation,
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
        Exit,
        IconView,
        ListView
    };

    FileMenuManager();

    static DFileMenu *createFileMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createFolderMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createViewSpaceAreaMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createRecentLeftBarMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createRecentFileMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createRecentViewSpaceAreaMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createDefaultBookMarkMenu();
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
    static QString checkDuplicateName(const QString &name);
    static QVector<FileMenuManager::MenuAction> getDisableActionList(const DUrl &fileUrl);

private:
    static void initData();
    static void initActions();
    static DFileMenu *genereteMenuByKeys(const QVector<MenuAction> &keys,
                                         const QVector<MenuAction> &disableList,
                                         bool checkable = false,
                                         const QMap<MenuAction, QVector<MenuAction> > &subMenuList = QMap<MenuAction, QVector<MenuAction> >());

    static QMap<MenuAction, QString> m_actionKeys;
    static QMap<MenuAction, DAction*> m_actions;

public slots:
    void actionTriggered(DAction * action);

signals:

};

#endif // FILEMENUMANAGER_H
