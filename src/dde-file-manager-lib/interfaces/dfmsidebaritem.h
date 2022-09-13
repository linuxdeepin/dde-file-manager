// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DStandardItem>

#include "durl.h"
#include "dfmglobal.h"

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMSideBarItem : public DStandardItem
{
public:
    enum Roles {
        ItemTypeRole = Dtk::UserRole + 1,
        ItemUrlRole,
        ItemGroupNameRole,
        ItemUseRegisteredHandlerRole,
        ItemUniqueKeyRole,
        ItemVolTagRole,
        ItemSmbMountedUrls,
        ItemReportNameRole,
        ItemUserCustomRole = Dtk::UserRole + 0x0100
    };

    enum ItemTypes {
        SidebarItem,
        Separator
    };

    enum CdAction {
        ChangeDirectory,
        MountPartitionThenCd,
        UserCustom = 0x0100
    };

    DFMSideBarItem(const DUrl &url = DUrl(), const QString &groupName = "default") : DFMSideBarItem (QIcon(), QString(), url, groupName) {}
    DFMSideBarItem(const QIcon &icon, const QString &text, const DUrl &url = DUrl(), const QString &groupName = "default");

    static DFMSideBarItem *createSeparatorItem(const QString &groupName);

    DUrl url() const;
    QString groupName() const;
    int itemType() const;
    int type() const;
    bool useRegisteredHandler() const;
    QString registeredHandler(const QString &fallback = QString()) const;

    void setUrl(const DUrl &url);
    void setGroupName(const QString &groupName);
    void setRegisteredHandler(const QString &identifier);
    void setReportName(const QString &reportName);

    constexpr static int SidebarItemType = QStandardItem::UserType + 1;

private:
    void initModelData();
};

DFM_END_NAMESPACE
