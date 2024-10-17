// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABBARMANAGER_H
#define TABBARMANAGER_H

#include "dfmplugin_titlebar_global.h"

#include <QObject>
#include <QHash>

namespace dfmplugin_titlebar {

class TabBar;
class TabBarManager : public QObject
{
    Q_OBJECT
public:
    ~TabBarManager() override;

    static TabBarManager *instance();

    TabBar *createTabBar(quint64 windowId, QWidget *parent);
    void removeTabBar(const quint64 windowId);

    void setCurrentUrl(QWidget *parent, const QUrl &url);

    void openNewTab(const quint64 windowId, const QUrl &url);
    bool canAddNewTab(const quint64 windowId);
    void closeTab(const QUrl &url);
    void setTabAlias(const QUrl &url, const QString &newName);
    void closeCurrentTab(const quint64 windowId);
    void activateNextTab(const quint64 windowId);
    void activatePreviousTab(const quint64 windowId);
    void createNewTab(const quint64 windowId);
    void activateTab(const quint64 windowId, const int index);
    void checkCurrentTab(const quint64 windowId);

private:
    explicit TabBarManager(QObject *parent = nullptr);
    QUrl getCurrentUrl(const quint64 windowId);

    QHash<quint64, TabBar *> tabBars {};
    QHash<QWidget *, TabBar *> tabBarsByWidget {};
};

} // namespace dfmplugin_titlebar

#endif // TABBARMANAGER_H
