#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include "abstractfileinfo.h"

#include <DAction>

#include <QObject>
#include <QMap>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

DWIDGET_USE_NAMESPACE

class DFileMenu;
class DUrl;

typedef AbstractFileInfo::MenuAction MenuAction;

class FileMenuManager : public QObject
{
    Q_OBJECT

public:
    FileMenuManager();

    static DFileMenu *createRecentLeftBarMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createDefaultBookMarkMenu();
    static DFileMenu *createCustomBookMarkMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createTrashLeftBarMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createDiskLeftBarMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createDiskViewMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createToolBarSettingsMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createToolBarSortMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static DFileMenu *createListViewHeaderMenu(const QVector<MenuAction> &disableList = QVector<MenuAction>());
    static QString checkDuplicateName(const QString &name);
    static QVector<MenuAction> getDisableActionList(const DUrl &fileUrl);

    static DFileMenu *genereteMenuByKeys(const QVector<MenuAction> &keys,
                                         const QVector<MenuAction> &disableList,
                                         bool checkable = false,
                                         const QMap<MenuAction, QVector<MenuAction> > &subMenuList = QMap<MenuAction, QVector<MenuAction> >());

private:
    static void initData();
    static void initActions();

    static QMap<MenuAction, QString> m_actionKeys;
    static QMap<MenuAction, DAction*> m_actions;
    static QVector<MenuAction> m_sortActionTypes;

public slots:
    void actionTriggered(DAction * action);
    void checkSortMenu(MenuAction type);

signals:

};

#endif // FILEMENUMANAGER_H
