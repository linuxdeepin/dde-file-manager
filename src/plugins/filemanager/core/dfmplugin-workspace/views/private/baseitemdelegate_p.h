// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEITEMDELEGATE_P_H
#define BASEITEMDELEGATE_P_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/utils/elidetextlayout.h>

#include <QModelIndex>
#include <QSize>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class AbstractItemPaintProxy;
class FileViewHelper;
class BaseItemDelegate;
class BaseItemDelegatePrivate
{
public:
    explicit BaseItemDelegatePrivate(BaseItemDelegate *qq);
    virtual ~BaseItemDelegatePrivate();

    void init();

    int textLineHeight { -1 };
    QSize itemSizeHint;
    mutable QModelIndex editingIndex;
    mutable QLineEdit *editor = nullptr;

    AbstractItemPaintProxy *paintProxy { nullptr };

    BaseItemDelegate *q_ptr;
    Q_DECLARE_PUBLIC(BaseItemDelegate)
};

}

#endif   // BASEITEMDELEGATE_P_H
