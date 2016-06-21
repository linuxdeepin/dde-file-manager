#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include "abstractfileinfo.h"

#include <DAction>

#include <QObject>
#include <QMap>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDir>
#include <QSet>

DWIDGET_USE_NAMESPACE

class DFileMenu;
class DUrl;

class FileMenuManager : public QObject
{
    Q_OBJECT

public:

    FileMenuManager();

    static DFileMenu *createRecentLeftBarMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createDefaultBookMarkMenu();
    static DFileMenu *createNetworkMarkMenu();
    static DFileMenu *createCustomBookMarkMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createTrashLeftBarMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createDiskLeftBarMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createDiskViewMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createToolBarSettingsMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createToolBarSortMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createListViewHeaderMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static QString checkDuplicateName(const QString &name);
    static QSet<MenuAction> getDisableActionList(const DUrl &fileUrl);
    static QSet<MenuAction> getDisableActionList(const DUrlList &urlList);

    static DFileMenu *genereteMenuByKeys(const QVector<MenuAction> &keys,
                                         const QSet<MenuAction> &disableList,
                                         bool checkable = false,
                                         const QMap<MenuAction, QVector<MenuAction> > &subMenuList = QMap<MenuAction, QVector<MenuAction> >());
    static QString getActionString(MenuAction type);

private:
    static void initData();
    static void initActions();

    static QMap<MenuAction, QString> m_actionKeys;
    static QMap<MenuAction, DAction*> m_actions;
    static QVector<MenuAction> m_sortActionTypes;

public slots:
    void actionTriggered(DAction * action);

signals:

};

#endif // FILEMENUMANAGER_H
