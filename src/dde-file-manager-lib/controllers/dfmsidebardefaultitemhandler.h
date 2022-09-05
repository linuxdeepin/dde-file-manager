// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dfmsidebariteminterface.h"

#define SIDEBAR_ID_DEFAULT "__default"

DFM_BEGIN_NAMESPACE

class DFMSideBarDefaultItemHandler : public DFMSideBarItemInterface
{
public:
    static DFMSideBarItem * createItem(const QString &pathKey);

    explicit DFMSideBarDefaultItemHandler(QObject *parent = nullptr);

    void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    QMenu * contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
};

DFM_END_NAMESPACE
