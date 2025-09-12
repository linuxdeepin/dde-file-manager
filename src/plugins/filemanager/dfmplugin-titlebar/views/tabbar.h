// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABBAR_H
#define TABBAR_H

#include "dfmplugin_titlebar_global.h"

#include <DTabBar>

#include <QUrl>

namespace dfmplugin_titlebar {

struct Tab
{
    QUrl tabUrl;
    QString tabAlias;
    QString uniqueId;
};

class TabBarPrivate;
class TabBar : public DTK_WIDGET_NAMESPACE::DTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);
    ~TabBar() override;

    int createTab();
    void removeTab(const int index);
    void setCurrentUrl(const QUrl &url);
    void closeTab(const QUrl &url);

public Q_SLOTS:
    void activateNextTab();
    void activatePreviousTab();
    void updateTabName(int index);

Q_SIGNALS:
    void currentTabChanged(int oldIndex, int newIndex);
    void requestNewWindow(const QUrl &url);
    void newTabCreated(const QString &uniqueId);
    void tabHasRemoved(int oldIndex, int nextIndex);

protected:
    void paintTab(QPainter *painter, int index, const QStyleOptionTab &option) const override;
    QSize tabSizeHint(int index) const override;
    QSize minimumTabSizeHint(int index) const override;
    QSize maximumTabSizeHint(int index) const override;
    QMimeData *createMimeDataFromTab(int index, const QStyleOptionTab &option) const override;
    QPixmap createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const override;
    bool canInsertFromMimeData(int index, const QMimeData *source) const override;

    bool eventFilter(QObject *obj, QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    TabBarPrivate *const d;
    static QPixmap *sm_pDragPixmap;
};
}

#endif   // TABBAR_H
