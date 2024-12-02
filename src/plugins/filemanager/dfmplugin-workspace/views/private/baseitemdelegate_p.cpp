// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseitemdelegate_p.h"
#include "views/baseitemdelegate.h"
#include "views/fileview.h"
#include "utils/fileviewhelper.h"

#include <QPainter>
#include <QAbstractItemView>

using namespace dfmplugin_workspace;
using namespace dfmbase;

BaseItemDelegatePrivate::BaseItemDelegatePrivate(BaseItemDelegate *qq)
    : q_ptr(qq)
{
}

BaseItemDelegatePrivate::~BaseItemDelegatePrivate()
{
}

void BaseItemDelegatePrivate::init()
{
    Q_Q(BaseItemDelegate);

    q->connect(q, &BaseItemDelegate::commitData, q->parent(), &FileViewHelper::handleCommitData);
    q->connect(q->parent()->parent(), &QAbstractItemView::iconSizeChanged, q, &BaseItemDelegate::updateItemSizeHint);
}
