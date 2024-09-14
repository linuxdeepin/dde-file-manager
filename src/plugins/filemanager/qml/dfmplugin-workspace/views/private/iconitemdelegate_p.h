// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONITEMDELEGATE_P_H
#define ICONITEMDELEGATE_P_H

#include <dfm-base/dfm_base_global.h>
#include "baseitemdelegate_p.h"
#include "views/iconitemdelegate.h"
#include "views/expandedItem.h"

#include <QPointer>

namespace dfmplugin_workspace {

class IconItemDelegate;
class IconItemDelegatePrivate : public BaseItemDelegatePrivate
{
public:
    explicit IconItemDelegatePrivate(IconItemDelegate *qq);
    ~IconItemDelegatePrivate();

    QIcon checkedIcon = QIcon::fromTheme("emblem-checked");
    QSize itemIconSize;

    QPointer<ExpandedItem> expandedItem;
    mutable QModelIndex lastAndExpandedIndex;
    mutable QModelIndex expandedIndex;

    // default icon size is 64px.
    int currentIconSizeIndex { 1 };

    QTextDocument *document { nullptr };
    Q_DECLARE_PUBLIC(IconItemDelegate)
};

}

#endif   // ICONITEMDELEGATE_P_H
