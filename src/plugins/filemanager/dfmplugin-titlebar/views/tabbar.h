// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABBAR_H
#define TABBAR_H

#include "dfmplugin_titlebar_global.h"

#include <DTabBar>

#include <QUrl>

namespace dfmplugin_titlebar {

class TabBarPrivate;
class TabBar : public DTK_WIDGET_NAMESPACE::DTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);
    ~TabBar() override;

    int createTab();
    int createInactiveTab(const QUrl &url, const QVariantMap &userData = {});
    void removeTab(int index, int selectIndex = -1);
    void setCurrentUrl(const QUrl &url);
    void closeTab(const QUrl &url);

    bool isTabValid(int index) const;
    QUrl tabUrl(int index) const;
    QString tabAlias(int index) const;
    void setTabAlias(int index, const QString &alias);
    QString tabUniqueId(int index) const;
    QVariant tabUserData(int index, const QString &key) const;
    void setTabUserData(int index, const QString &key, const QVariant &userData);
    bool isInactiveTab(int index) const;
    bool isPinned(int index) const;

public Q_SLOTS:
    void activateNextTab();
    void activatePreviousTab();
    void updateTabName(int index);

Q_SIGNALS:
    void currentTabChanged(int oldIndex, int newIndex);
    void requestNewWindow(const QUrl &url);
    void newTabCreated();
    void requestCreateView(const QString &uniqueId);
    void tabAboutToRemove(int oldIndex, int nextIndex);

protected:
    void paintTab(QPainter *painter, int index, const QStyleOptionTab &option) const override;
    QSize tabSizeHint(int index) const override;
    QSize minimumTabSizeHint(int index) const override;
    QSize maximumTabSizeHint(int index) const override;
    QMimeData *createMimeDataFromTab(int index, const QStyleOptionTab &option) const override;
    QPixmap createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const override;
    bool canInsertFromMimeData(int index, const QMimeData *source) const override;
    void insertFromMimeData(int index, const QMimeData *source) override;
    void insertFromMimeDataOnDragEnter(int index, const QMimeData *source) override;

    bool eventFilter(QObject *obj, QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    TabBarPrivate *const d;
};
}

#endif   // TABBAR_H
