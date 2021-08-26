/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include "dfm-base/base/dfmglobal.h"

#include <DStandardItem>

#include <QUrl>

DWIDGET_USE_NAMESPACE

class DFMSideBarItem : public DStandardItem
{
public:
    enum Roles {
        ItemUrlRole = Dtk::UserRole + 1,
        ItemGroupRole,
        ItemUserCustomRole = Dtk::UserRole + 0x0100
    };

    DFMSideBarItem(const QUrl &url);

    DFMSideBarItem(const QIcon &icon,
                   const QString &text,
                   const QString &group,
                   const QUrl &url);

    virtual ~DFMSideBarItem();

    DFMSideBarItem(const DFMSideBarItem& item);

    QUrl url() const;
    void setUrl(const QUrl &url);
    void setGroup(const QString &group = "");

    QString group() const;
};


class DFMSideBarItemSeparator :public DFMSideBarItem
{
public:
    explicit DFMSideBarItemSeparator();
    virtual ~DFMSideBarItemSeparator();
};
Q_DECLARE_METATYPE(DFMSideBarItemSeparator)

#endif //DFMSIDEBARITEM_H
