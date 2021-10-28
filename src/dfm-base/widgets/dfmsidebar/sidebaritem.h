/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#ifndef DFMSIDEBARITEM_H
#define DFMSIDEBARITEM_H
#pragma once

#include "dfm-base/dfm_base_global.h"

#include <DStandardItem>

#include <QUrl>

DWIDGET_USE_NAMESPACE
DFMBASE_BEGIN_NAMESPACE
class SideBarItem : public DStandardItem
{
public:
    enum Roles {
        ItemUrlRole = Dtk::UserRole + 1,
        ItemGroupRole,
        ItemUserCustomRole = Dtk::UserRole + 0x0100
    };

    explicit SideBarItem(const QUrl &url);
    explicit SideBarItem(const QIcon &icon,
                         const QString &text,
                         const QString &group,
                         const QUrl &url);
    explicit SideBarItem(const SideBarItem& item);
    virtual ~SideBarItem();

    void setUrl(const QUrl &url);
    void setGroup(const QString &group = "");
    QUrl url() const;
    QString group() const;
};

class DFMSideBarItemSeparator :public SideBarItem
{
public:
    explicit DFMSideBarItemSeparator();
    virtual ~DFMSideBarItemSeparator();
};
DFMBASE_END_NAMESPACE
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::DFMSideBarItemSeparator)

#endif //DFMSIDEBARITEM_H
