// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include "dfmplugin_sidebar_global.h"

#include <dfm-base/interfaces/abstractframe.h>

#include <QUrl>

class QAbstractItemView;

DPSIDEBAR_BEGIN_NAMESPACE
class SideBarView;
class SideBarModel;
class SideBarItem;
class SideBarWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT

public:
    explicit SideBarWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &sidebarUrl) override;
    QUrl currentUrl() const override;
    void changeEvent(QEvent *event) override;

    QAbstractItemView *view();

    int addItem(SideBarItem *item);
    bool insertItem(const int index, SideBarItem *item);
    bool removeItem(SideBarItem *item);
    bool removeItem(const QUrl &url);

    void updateItem(const QUrl &url, const ItemInfo &newInfo);
    int findItem(const QUrl &url) const;
    void editItem(const QUrl &url);
    void setItemVisiable(const QUrl &url, bool visible);
    void updateItemVisiable(const QVariantMap &states);
    QList<QUrl> findItems(const QString &group) const;
    void updateSelection();
    void saveStateWhenClose();

private Q_SLOTS:
    void onItemActived(const QModelIndex &index);
    void customContextMenuCall(const QPoint &pos);
    void onItemRenamed(const QModelIndex &index, const QString &newName);

private:
    void initializeUi();
    void initDefaultModel();
    void initConnect();
    void updateSeparatorVisibleState();

private:
    QUrl sidebarUrl;
    SideBarView *sidebarView { nullptr };
    static QSharedPointer<SideBarModel> kSidebarModelIns;
    QStringList currentGroups;
};
DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARWIDGET_H
