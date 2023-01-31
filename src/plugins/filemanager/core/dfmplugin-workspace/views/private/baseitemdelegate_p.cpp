/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

ElideTextLayout *BaseItemDelegatePrivate::createTextlayout(const QModelIndex &index, const QPainter *painter) const
{
    bool showSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();
    QString name = showSuffix ? index.data(Global::ItemRoles::kItemFileDisplayNameRole).toString()
                              : index.data(Global::ItemRoles::kItemFileBaseNameOfRenameRole).toString();
    ElideTextLayout *layout = new ElideTextLayout(name);
    layout->setAttribute(ElideTextLayout::kWrapMode, (uint)QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout->setAttribute(ElideTextLayout::kLineHeight, textLineHeight);
    layout->setAttribute(ElideTextLayout::kAlignment, Qt::AlignCenter);

    if (painter) {
        layout->setAttribute(ElideTextLayout::kFont, painter->font());
        layout->setAttribute(ElideTextLayout::kTextDirection, painter->layoutDirection());
    }

    return layout;
}
