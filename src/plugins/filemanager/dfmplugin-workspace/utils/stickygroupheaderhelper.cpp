// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stickygroupheaderhelper.h"
#include "views/fileview.h"
#include "views/private/fileview_p.h"
#include "models/fileviewmodel.h"
#include "views/baseitemdelegate.h"

#include <dfm-base/dfm_global_defines.h>

#include <QStyleOptionViewItem>
#include <QPainter>
#include <QTimer>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

StickyGroupHeaderHelper::StickyGroupHeaderHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

int StickyGroupHeaderHelper::groupHeaderContentTop(const QModelIndex &index) const
{
    int top = view->visualRect(index).top();
    if (index.data(Global::kItemGroupDisplayIndex).toInt() > 0)
        top += kGroupHeaderInterval;
    return top;
}

int StickyGroupHeaderHelper::stickyHeaderHeight() const
{
    if (!view->itemDelegate())
        return 0;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QStyleOptionViewItem opt = view->viewOptions();
#else
    QStyleOptionViewItem opt;
    view->initViewItemOption(&opt);
#endif
    return view->itemDelegate()->getGroupHeaderHeight(opt);
}

QModelIndex StickyGroupHeaderHelper::findStickyGroupIndex(int headerHeight) const
{
    if (!view->isGroupedView() || !view->model())
        return QModelIndex();

    const int rowCount = view->model()->rowCount(view->rootIndex());
    if (rowCount == 0 || view->model()->groupingState() != GroupingState::kIdle)
        return QModelIndex();

    if (view->d->currentStickyIndex.isValid() && view->isGroupHeader(view->d->currentStickyIndex)
        && groupHeaderContentTop(view->d->currentStickyIndex) < 0) {
        if (!view->d->cachedNextStickyHeader.isValid())
            return view->d->currentStickyIndex;
        if (view->isGroupHeader(view->d->cachedNextStickyHeader)
            && groupHeaderContentTop(view->d->cachedNextStickyHeader) >= 0)
            return view->d->currentStickyIndex;
    }

    QModelIndex firstVisible;
    if (view->isIconViewMode()) {
        firstVisible = view->iconIndexAt(QPoint(0, 1), view->itemSizeHint());
    } else {
        firstVisible = view->DListView::indexAt(QPoint(0, 1));
    }

    if (!firstVisible.isValid()) {
        for (int i = 0; i < rowCount; ++i) {
            QModelIndex idx = view->model()->index(i, 0, view->rootIndex());
            QRect rect = view->visualRect(idx);
            if (rect.bottom() >= 0) {
                firstVisible = idx;
                break;
            }
            if (rect.top() > 0)
                break;
        }
    }

    if (!firstVisible.isValid())
        return QModelIndex();

    QModelIndex sticky;
    for (int i = firstVisible.row(); i >= 0; --i) {
        QModelIndex idx = view->model()->index(i, 0, view->rootIndex());
        if (view->isGroupHeader(idx)) {
            int contentTop = groupHeaderContentTop(idx);
            if (contentTop >= headerHeight)
                return QModelIndex();
            if (contentTop >= 0) {
                for (int j = idx.row() - 1; j >= 0; --j) {
                    QModelIndex prevIdx = view->model()->index(j, 0, view->rootIndex());
                    if (view->isGroupHeader(prevIdx)) {
                        sticky = prevIdx;
                        break;
                    }
                }
            } else {
                sticky = idx;
            }
            break;
        }
    }

    // Cache the next group header so computeStickyY is O(1).
    view->d->cachedNextStickyHeader = QModelIndex();
    if (sticky.isValid()) {
        for (int i = sticky.row() + 1; i < rowCount; ++i) {
            QModelIndex idx = view->model()->index(i, 0, view->rootIndex());
            if (view->isGroupHeader(idx)) {
                view->d->cachedNextStickyHeader = idx;
                break;
            }
        }
    }
    return sticky;
}

int StickyGroupHeaderHelper::computeStickyY(int headerHeight) const
{
    if (!view->d->cachedNextStickyHeader.isValid())
        return 0;

    int nextTop = view->visualRect(view->d->cachedNextStickyHeader).top();
    return qMax(-headerHeight, qMin(0, nextTop - headerHeight));
}

void StickyGroupHeaderHelper::paintStickyHeaderOverlay(const QModelIndex &index, int y, int headerHeight)
{
    if (!index.isValid() || !view->itemDelegate())
        return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QStyleOptionViewItem opt = view->viewOptions();
#else
    QStyleOptionViewItem opt;
    view->initViewItemOption(&opt);
#endif
    opt.widget = view->viewport();
    opt.rect = QRect(0, y, view->viewport()->width(), headerHeight);
    opt.index = index;

    if (view->d->stickyHeaderHovered)
        opt.state |= QStyle::State_MouseOver;
    else
        opt.state &= ~QStyle::State_MouseOver;

    QPainter painter(view->viewport());
    view->itemDelegate()->paintStickyGroupHeader(&painter, opt, index);

    view->d->currentStickyIndex = index;
    view->d->currentStickyRect = opt.rect;
}

bool StickyGroupHeaderHelper::isPosInStickyHeader(const QPoint &pos) const
{
    return view->d->currentStickyIndex.isValid() && view->d->currentStickyRect.contains(pos);
}

void StickyGroupHeaderHelper::clearStickyHeaderState()
{
    view->d->currentStickyIndex = QModelIndex();
    view->d->currentStickyRect = QRect();
    view->d->stickyHeaderHovered = false;
    view->d->cachedNextStickyHeader = QModelIndex();
}

void StickyGroupHeaderHelper::scrollStickyHeaderToTop(const QModelIndex &headerIndex)
{
    const int row = headerIndex.row();
    QTimer::singleShot(0, view, [this, row] {
        QModelIndex idx = view->model()->index(row, 0, view->rootIndex());
        if (idx.isValid())
            view->scrollTo(idx, QAbstractItemView::PositionAtTop);
    });
}
