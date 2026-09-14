// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseitemdelegate_p.h"
#include "views/baseitemdelegate.h"
#include "views/fileview.h"
#include "utils/fileviewhelper.h"

#include <QPainter>
#include <QAbstractItemView>
#include <QTextOption>

using namespace dfmplugin_workspace;
using namespace dfmbase;

BaseItemDelegatePrivate::BaseItemDelegatePrivate(BaseItemDelegate *qq)
    : q_ptr(qq)
{
    reusableElideLayout.reset(new ElideTextLayout);
}
BaseItemDelegatePrivate::~BaseItemDelegatePrivate() = default;

void BaseItemDelegatePrivate::setupElideLayout(dfmbase::ElideTextLayout *layout,
                                                const QString &text,
                                                QTextOption::WrapMode wrapMode,
                                                int lineHeight,
                                                int alignment,
                                                QPainter *painter,
                                                bool highlightEnabled,
                                                const QStringList &keywords,
                                                const QColor &highlightColor) const
{
    // 复用 ElideTextLayout 前必须重置所有持久属性，避免上一帧残留。
    // kBackgroundRadius 仅在 IconItemDelegate 选中时设置，若不重置会导致
    // 拖拽场景下文件名背景圆角错误（状态泄漏）。
    layout->setAttribute(ElideTextLayout::kBackgroundRadius, 0);
    layout->setText(text);
    layout->setAttribute(ElideTextLayout::kWrapMode, wrapMode);
    layout->setAttribute(ElideTextLayout::kLineHeight, lineHeight);
    layout->setAttribute(ElideTextLayout::kAlignment, alignment);
    layout->setAttribute(ElideTextLayout::kFont, painter->font());
    layout->setAttribute(ElideTextLayout::kTextDirection, painter->layoutDirection());
    layout->setHighlightEnabled(highlightEnabled);
    layout->setHighlightKeywords(keywords);
    layout->setHighlightColor(highlightColor);
}

void BaseItemDelegatePrivate::init()
{
    Q_Q(BaseItemDelegate);

    q->connect(q, &BaseItemDelegate::commitData, q->parent(), &FileViewHelper::handleCommitData);
    auto *view = q->parent() ? q->parent()->parent() : nullptr;
    if (view)
        q->connect(view, &QAbstractItemView::iconSizeChanged, q, &BaseItemDelegate::updateItemSizeHint);
}
