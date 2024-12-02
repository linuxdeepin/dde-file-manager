// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSIDEBARITEM_H
#define DFMSIDEBARITEM_H

#include "dfmplugin_sidebar_global.h"

#include <DStandardItem>
#include <QUrl>

DWIDGET_USE_NAMESPACE

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarItem : public DStandardItem
{
public:
    enum Roles {
        kItemUrlRole = Dtk::UserRole + 1,
        kItemGroupRole,
        kItemTypeRole,
        kItemHiddenRole,
        kItemUserCustomRole = Dtk::UserRole + 0x0100
    };

    enum ItemTypes {
        kSidebarItem,
        kSeparator
    };

    explicit SideBarItem(const QUrl &url);
    explicit SideBarItem(const QIcon &icon,
                         const QString &text,
                         const QString &group,
                         const QUrl &url);
    explicit SideBarItem(const SideBarItem &item);
    virtual ~SideBarItem();

    QUrl url() const;
    QUrl targetUrl() const;
    void setUrl(const QUrl &url);

    QString group() const;
    void setGroup(const QString &group = "");
    QString subGourp() const;

    bool isHidden() const;
    void setHiiden(bool hidden);

    ItemInfo itemInfo() const;
};

class SideBarItemSeparator : public DPSIDEBAR_NAMESPACE::SideBarItem
{
public:
    explicit SideBarItemSeparator(const QString &group = "");
    virtual ~SideBarItemSeparator();
    void setExpanded(bool state);
    bool isExpanded();
    void setVisible(bool value);
    bool isVisible();

private:
    bool expanded = true;   // TODO(zhuangshu): Maybe it's better to store it to `ItemInfo`, do it later
    bool visible = true;
};

DPSIDEBAR_END_NAMESPACE

Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::SideBarItemSeparator)

#endif   //DFMSIDEBARITEM_H
