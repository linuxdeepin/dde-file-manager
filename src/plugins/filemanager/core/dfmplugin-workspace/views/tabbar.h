// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABBAR_H
#define TABBAR_H

#include <dfm-base/dfm_base_global.h>
#include "dfmplugin_workspace_global.h"

#include <QGraphicsView>
#include <QGraphicsScene>

namespace dfmbase {
class AbstractBaseView;
}

namespace dfmplugin_workspace {

class Tab;
class TabCloseButton;
class TabBar : public QGraphicsView
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);
    ~TabBar() override;

    int createTab();
    void removeTab(const int index, const bool &remainState = false);
    Tab *currentTab() const;
    Tab *tabAt(const int &index);
    int getCurrentIndex() const;
    bool tabAddable() const;
    void setCurrentIndex(const int index);
    void setCurrentUrl(const QUrl &url);
    void closeTab(quint64 winId, const QUrl &url);
    int count() const;

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

}

#endif   // TABBAR_H
