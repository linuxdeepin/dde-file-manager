// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dfmsidebariteminterface.h"
#include "dtkwidget_global.h"

#define SIDEBAR_ID_DEVICE "__device"

DWIDGET_BEGIN_NAMESPACE
class DViewItemAction;
DWIDGET_END_NAMESPACE

class DUrl;

DFM_BEGIN_NAMESPACE

class DFMSideBarDeviceItemHandler : public DFMSideBarItemInterface
{
public:
    static DTK_WIDGET_NAMESPACE::DViewItemAction * createUnmountOrEjectAction(const DUrl &url, bool withText);
    static DFMSideBarItem * createItem(const DUrl &url);

    explicit DFMSideBarDeviceItemHandler(QObject *parent = nullptr);

    void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    QMenu * contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    void rename(const DFMSideBarItem *item, QString name) override;

    static QString reportName(const DUrl &url);
};

DFM_END_NAMESPACE
