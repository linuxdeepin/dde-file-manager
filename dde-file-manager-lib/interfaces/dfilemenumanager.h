#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include "dfmglobal.h"

#include <DAction>

#include <QObject>
#include <QMap>
#include <QSet>

DWIDGET_USE_NAMESPACE

class DFileMenu;
class DUrl;
typedef DFMGlobal::MenuAction MenuAction;
typedef QList<DUrl> DUrlList;

class DFileMenuManager : public QObject
{
    Q_OBJECT

public:
    DFileMenuManager();

    static DFileMenu *createRecentLeftBarMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createDefaultBookMarkMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createNetworkMarkMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createUserShareMarkMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createCustomBookMarkMenu(const DUrl& url, QSet<MenuAction> disableList = QSet<MenuAction>());
    static DFileMenu *createTrashLeftBarMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createComputerLeftBarMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createDiskLeftBarMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createDiskViewMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createToolBarSettingsMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createToolBarSortMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createListViewHeaderMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());

    static DFileMenu *createNormalMenu(const DUrl &currentUrl, const DUrlList &urlList, QSet<MenuAction> disableList, QSet<MenuAction> unusedList, int windowId);

    static void loadNormalPluginMenu(DFileMenu* menu, const DUrlList &urlList);
    static void loadEmptyPluginMenu(DFileMenu* menu);

    static QString checkDuplicateName(const QString &name);
    static QSet<MenuAction> getDisableActionList(const DUrl &fileUrl);
    static QSet<MenuAction> getDisableActionList(const DUrlList &urlList);

    static DFileMenu *genereteMenuByKeys(const QVector<MenuAction> &keys,
                                         const QSet<MenuAction> &disableList,
                                         bool checkable = false,
                                         const QMap<MenuAction, QVector<MenuAction> > &subMenuList = QMap<MenuAction, QVector<MenuAction> >());
    static QString getActionString(MenuAction type);

    /// actions filter(global)
    static void addActionWhitelist(MenuAction action);
    static void setActionWhitelist(const QSet<MenuAction> &actionList);
    static QSet<MenuAction> actionWhitelist();
    static void addActionBlacklist(MenuAction action);
    static void setActionBlacklist(const QSet<MenuAction> &actionList);
    static QSet<MenuAction> actionBlacklist();
    static bool isAvailableAction(MenuAction action);

    static DAction* qActionToDAction(QAction* action, DAction* parentAction=NULL, DMenu* dMenu=NULL);

public slots:
    void actionTriggered(DAction * action);
};

#endif // FILEMENUMANAGER_H
