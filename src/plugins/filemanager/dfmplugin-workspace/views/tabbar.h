/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef TABBAR_H
#define TABBAR_H

#include "dfm-base/dfm_base_global.h"
#include "dfmplugin_workspace_global.h"

#include <QGraphicsView>
#include <QGraphicsScene>

DFMBASE_BEGIN_NAMESPACE
class AbstractBaseView;
DFMBASE_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

class Tab;
class TabCloseButton;
class TabBar : public QGraphicsView
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);

    int createTab(DFMBASE_NAMESPACE::AbstractBaseView *view);
    void removeTab(const int index, const bool &remainState = false);
    Tab *currentTab() const;
    Tab *tabAt(const int &index);
    int getCurrentIndex() const;
    bool tabAddable() const;
    void setCurrentIndex(const int index);
    void setCurrentUrl(const QUrl &url);
    void setCurrentView(DFMBASE_NAMESPACE::AbstractBaseView *view);
    void closeTab(quint64 winId, const QUrl &url);

public slots:
    void onTabCloseButtonHovered(int closingIndex);
    void onTabCloseButtonUnHovered(int closingIndex);
    void onTabCloseButtonClicked();
    void onMoveNext(Tab *tab);
    void onMovePrevius(Tab *tab);
    void onRequestNewWindow(const QUrl url);
    void onAboutToNewWindow(Tab *tab);
    void onTabClicked();
    void onTabDragFinished();
    void onTabDragStarted();
    void activateNextTab();
    void activatePreviousTab();

protected slots:
    void closeTabAndRemoveCachedMnts(const QString &id);
    void cacheMnt(const QString &id, const QString &mnt);

signals:
    void currentChanged(const int &index);
    void tabCloseRequested(const int &index, const bool &remainState = false);
    void tabAddableChanged(const bool &tabAddable);
    void tabMoved(const int &from, const int &to);
    void tabBarShown();
    void tabBarHidden();
    void requestNewWindow(const QUrl &url);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void initializeUI();
    void initializeConnections();
    void updateScreen();
    QSize tabSizeHint(const int &index);
    void setTabText(const int &index, const QString &text);
    int count() const;
    void handleTabAnimationFinished(const int index);
    bool isMountedDevPath(const QUrl &url);

    QGraphicsScene *scene = nullptr;
    QList<Tab *> tabList;
    TabCloseButton *tabCloseButton = nullptr;

    bool lastDeleteState = false;
    bool lastAddTabState = false;
    bool barHovered = false;
    bool lastStateOndelete = false;
    int lastDeletedAverageWidth = 0;
    QPoint tabLastMouseDragPos;
    bool tabDragging = false;
    int trackingIndex = 0;
    int currentIndex = -1;
    int historyWidth = 0;

    QMultiHash<QString, QUrl> allMntedDevs;
};

DPWORKSPACE_END_NAMESPACE

#endif   // TABBAR_H
