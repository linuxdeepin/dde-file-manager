/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include "dfmglobal.h"

#include <QAction>

#include <QObject>
#include <QMap>
#include <QSet>

class DFileMenu;
class DUrl;
typedef DFMGlobal::MenuAction MenuAction;
typedef QList<DUrl> DUrlList;

class DFileMenuManager : public QObject
{
    Q_OBJECT

public:
    DFileMenuManager();

    static DFileMenu *createDefaultBookMarkMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createUserShareMarkMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());
    static DFileMenu *createToolBarSettingsMenu(const QSet<MenuAction> &disableList = QSet<MenuAction>());

    static DFileMenu *createNormalMenu(const DUrl &currentUrl, const DUrlList &urlList, QSet<MenuAction> disableList, QSet<MenuAction> unusedList, int windowId);

    static QList<QAction *> loadNormalPluginMenu(DFileMenu *menu, const DUrlList &urlList, const DUrl &currentUrl);
    static QList<QAction *> loadNormalExtensionMenu(DFileMenu *menu, const DUrlList &urlList, const DUrl &currentUrl);

    static QList<QAction *> loadEmptyAreaPluginMenu(DFileMenu *menu, const DUrl &currentUrl, bool onDesktop);
    static QList<QAction *> loadEmptyAreaExtensionMenu(DFileMenu *menu, const DUrl &currentUrl, bool onDesktop);

    static QList<QAction *> loadMenuExtensionActions(const DUrlList &urlList, const DUrl &currentUrl, bool onDesktop = false);
    static QList<QAction *> jsonToActions(const QJsonArray &data, const DUrlList &urlList, const DUrl &currentUrl,
                                          const QString &menuExtensionType, const bool onDesktop);

    static QAction *getAction(MenuAction action);

    static QString checkDuplicateName(const QString &name);
    static QSet<MenuAction> getDisableActionList(const DUrl &fileUrl);
    static QSet<MenuAction> getDisableActionList(const DUrlList &urlList);

    static DFileMenu *genereteMenuByKeys(const QVector<MenuAction> &keys,
                                         const QSet<MenuAction> &disableList,
                                         bool checkable = false,
                                         const QMap<MenuAction, QVector<MenuAction> > &subMenuList = QMap<MenuAction, QVector<MenuAction> >(),
                                         bool isUseCachedAction = true,
                                         bool isRecursiveCall = false);
    static QString getActionString(MenuAction type);

    /// actions filter(global)
    static void addActionWhitelist(MenuAction action);
    static void setActionWhitelist(const QSet<MenuAction> &actionList);
    static QSet<MenuAction> actionWhitelist();
    static void addActionBlacklist(MenuAction action);
    static void setActionBlacklist(const QSet<MenuAction> &actionList);
    static QSet<MenuAction> actionBlacklist();
    static bool isAvailableAction(MenuAction action);

    static QList<QMap<QString, QString>> ExtensionMenuItems;


    static void setActionString(MenuAction type, QString actionString);
    static void setActionID(MenuAction type, QString id);

    static MenuAction registerMenuActionType(QAction *action);

    static bool whetherShowTagActions(const QList<DUrl> &urls);

public slots:
    void actionTriggered(QAction *action);
};

#endif // FILEMENUMANAGER_H
