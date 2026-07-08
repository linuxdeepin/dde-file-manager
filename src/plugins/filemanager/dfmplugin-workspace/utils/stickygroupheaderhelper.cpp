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
      m_view(parent)
{
}

int StickyGroupHeaderHelper::groupHeaderContentTop(const QModelIndex &index) const
{
    int top = m_view->visualRect(index).top();
    if (index.data(Global::kItemGroupDisplayIndex).toInt() > 0)
        top += kGroupHeaderInterval;
    return top;
}

int StickyGroupHeaderHelper::stickyHeaderHeight() const
{
    if (!m_view->itemDelegate())
        return 0;

    QStyleOptionViewItem opt;
    m_view->initViewItemOption(&opt);

    return m_view->itemDelegate()->getGroupHeaderHeight(opt);
}

QModelIndex StickyGroupHeaderHelper::findStickyGroupIndex(int headerHeight)
{
    if (!m_view->isGroupedView() || !m_view->model())
        return QModelIndex();

    const int rowCount = m_view->model()->rowCount(m_view->rootIndex());
    if (rowCount == 0 || m_view->model()->groupingState() != GroupingState::kIdle)
        return QModelIndex();

    if (m_currentStickyIndex.isValid() && m_view->isGroupHeader(m_currentStickyIndex)
        && groupHeaderContentTop(m_currentStickyIndex) < 0) {
        if (!m_cachedNextStickyHeader.isValid())
            return m_currentStickyIndex;
        if (m_view->isGroupHeader(m_cachedNextStickyHeader)
            && groupHeaderContentTop(m_cachedNextStickyHeader) >= 0)
            return m_currentStickyIndex;
    }

    QModelIndex firstVisible;
    if (m_view->isIconViewMode()) {
        firstVisible = m_view->iconIndexAt(QPoint(0, 1), m_view->itemSizeHint());
    } else {
        firstVisible = m_view->DListView::indexAt(QPoint(0, 1));
    }

    if (!firstVisible.isValid()) {
        for (int i = 0; i < rowCount; ++i) {
            QModelIndex idx = m_view->model()->index(i, 0, m_view->rootIndex());
            QRect rect = m_view->visualRect(idx);
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
        QModelIndex idx = m_view->model()->index(i, 0, m_view->rootIndex());
        if (m_view->isGroupHeader(idx)) {
            int contentTop = groupHeaderContentTop(idx);
            if (contentTop >= headerHeight)
                return QModelIndex();
            if (contentTop >= 0) {
                for (int j = idx.row() - 1; j >= 0; --j) {
                    QModelIndex prevIdx = m_view->model()->index(j, 0, m_view->rootIndex());
                    if (m_view->isGroupHeader(prevIdx)) {
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
    m_cachedNextStickyHeader = QModelIndex();
    if (sticky.isValid()) {
        for (int i = sticky.row() + 1; i < rowCount; ++i) {
            QModelIndex idx = m_view->model()->index(i, 0, m_view->rootIndex());
            if (m_view->isGroupHeader(idx)) {
                m_cachedNextStickyHeader = idx;
                break;
            }
        }
    }
    return sticky;
}

int StickyGroupHeaderHelper::computeStickyY(int headerHeight) const
{
    if (!m_cachedNextStickyHeader.isValid())
        return 0;

    int nextTop = m_view->visualRect(m_cachedNextStickyHeader).top();
    return qMax(-headerHeight, qMin(0, nextTop - headerHeight));
}

void StickyGroupHeaderHelper::paintStickyHeaderOverlay(const QModelIndex &index, int y, int headerHeight)
{
    if (!index.isValid() || !m_view->itemDelegate())
        return;

    QStyleOptionViewItem opt;
    m_view->initViewItemOption(&opt);

    opt.widget = m_view->viewport();
    opt.rect = QRect(0, y, m_view->viewport()->width(), headerHeight);
    opt.index = index;

    if (m_stickyHeaderHovered)
        opt.state |= QStyle::State_MouseOver;
    else
        opt.state &= ~QStyle::State_MouseOver;

    QPainter painter(m_view->viewport());
    m_view->itemDelegate()->paintStickyGroupHeader(&painter, opt, index);

    m_currentStickyIndex = index;
    m_currentStickyRect = opt.rect;
}

bool StickyGroupHeaderHelper::isPosInStickyHeader(const QPoint &pos) const
{
    return m_currentStickyIndex.isValid() && m_currentStickyRect.contains(pos);
}

void StickyGroupHeaderHelper::clearStickyHeaderState()
{
    m_currentStickyIndex = QModelIndex();
    m_currentStickyRect = QRect();
    m_stickyHeaderHovered = false;
    m_cachedNextStickyHeader = QModelIndex();
}

void StickyGroupHeaderHelper::scrollStickyHeaderToTop(const QModelIndex &headerIndex)
{
    const int row = headerIndex.row();
    QTimer::singleShot(0, m_view, [this, row] {
        QModelIndex idx = m_view->model()->index(row, 0, m_view->rootIndex());
        if (idx.isValid())
            m_view->scrollTo(idx, QAbstractItemView::PositionAtTop);
    });
}
