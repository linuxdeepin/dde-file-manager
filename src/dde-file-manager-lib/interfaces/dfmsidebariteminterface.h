// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMenu>
#include <QObject>
#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMSideBar;
class DFMSideBarItem;
class DFMSideBarItemInterface : public QObject
{
    Q_GADGET
public:
    explicit DFMSideBarItemInterface(QObject *parent = nullptr);

    virtual void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item);
    virtual QMenu * contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item);
    virtual void rename(const DFMSideBarItem *item, QString name);
};

DFM_END_NAMESPACE
