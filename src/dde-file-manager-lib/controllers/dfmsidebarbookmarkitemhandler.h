// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dfmsidebariteminterface.h"

#define SIDEBAR_ID_BOOKMARK "__bookmark"

class DUrl;

DFM_BEGIN_NAMESPACE

class DFMSideBarBookmarkItemHandler : public DFMSideBarItemInterface
{
public:
    static DFMSideBarItem * createItem(const DUrl &url);

    DFMSideBarBookmarkItemHandler();

    void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    QMenu * contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    void rename(const DFMSideBarItem *item, QString name) override;
};

DFM_END_NAMESPACE
