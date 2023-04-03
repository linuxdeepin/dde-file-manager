// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTITEMDELEGATE_P_H
#define LISTITEMDELEGATE_P_H

#include <dfm-base/dfm_base_global.h>
#include "baseitemdelegate_p.h"
#include "views/listitemdelegate.h"

#include <QObject>

namespace dfmplugin_workspace {

class ListItemDelegate;
class ListItemDelegatePrivate : public BaseItemDelegatePrivate
{
public:
    explicit ListItemDelegatePrivate(ListItemDelegate *qq);
    virtual ~ListItemDelegatePrivate();

    Q_DECLARE_PUBLIC(ListItemDelegate)
};

}

#endif   // LISTITEMDELEGATE_P_H
