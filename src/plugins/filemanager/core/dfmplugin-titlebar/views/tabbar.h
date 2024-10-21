// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABBAR_H
#define TABBAR_H

#include <dfm-base/dfm_base_global.h>

#include <DIconButton>

#include <QGraphicsView>
#include <QGraphicsScene>

namespace dfmplugin_titlebar {

class Tab;
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
    void closeTab(const QUrl &url);
    int count() const;

public Q_SLOTS:
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

Q_SIGNALS:
    void currentChanged(const int &index);
    void tabCloseRequested(const int &index, const bool &remainState = false);
    void requestNewWindow(const QUrl &url);
    void newTabCreated();
    void tabRemoved(int index);
    void tabMoved(int from, int to);
    void tabAddButtonClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void initializeUI();
    void initializeConnections();
    void updateScreen();
    QSize tabSizeHint(const int &index);
    void setTabText(const int &index, const QString &text);
    void handleTabAnimationFinished(const int index);
    void updateTabsState();
    void updateAddTabButtonState();

    inline int getTabAreaWidth() const {
        return width() - 10- tabAddButton->width();
    }

    QGraphicsScene *scene { nullptr };
    QList<Tab *> tabList {};
    DTK_WIDGET_NAMESPACE::DIconButton *tabAddButton { nullptr };

    bool playTabAnimation { true };
    bool barHovered { false };
    bool lastStateOndelete { false };
    int lastDeletedAverageWidth { 0 };
    QPoint tabLastMouseDragPos;
    bool tabDragging { false };
    int trackingIndex { 0 };
    int currentIndex { -1 };
    int historyWidth { 0 };

    QPoint dragStartPosition;
    bool isDragging { false };
};

} // namespace dfmplugin_titlebar

#endif // TABBAR_H
