/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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

#include "headerview.h"
#include "views/fileview.h"
#include "models/filesortfilterproxymodel.h"

#include <QApplication>
#include <QMouseEvent>

DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

HeaderView::HeaderView(Qt::Orientation orientation, FileView *parent)
    : QHeaderView(orientation, parent),
      view(parent)
{
    setHighlightSections(false);
    setSectionsClickable(true);
    setSortIndicatorShown(true);
    setSectionsMovable(true);
    setFirstSectionMovable(false);
}

QSize HeaderView::sizeHint() const
{
    QSize size = QHeaderView::sizeHint();

    size.setWidth(length());

    return size;
}

int HeaderView::sectionsTotalWidth() const
{
    int totalWidth = 0;
    for (int i = 0; i < model()->columnCount(); ++i) {
        totalWidth += sectionSize(i);
    }

    return totalWidth;
}

void HeaderView::updateColumnWidth()
{
    auto proxyModel = this->proxyModel();
    if (proxyModel) {
        int columnCount = count();
        int i = 0;
        int j = columnCount - 1;

        for (; i < columnCount; ++i) {
            int logicalIndex = this->logicalIndex(i);
            if (isSectionHidden(logicalIndex))
                continue;

            resizeSection(logicalIndex, proxyModel->getColumnWidth(i) + kLeftPadding + kListModeLeftMargin + 2 * kColumnPadding);
            break;
        }

        for (; j > 0; --j) {
            int logicalIndex = this->logicalIndex(j);
            if (isSectionHidden(logicalIndex))
                continue;

            resizeSection(logicalIndex, proxyModel->getColumnWidth(j) + kRightPadding + kListModeRightMargin + 2 * kColumnPadding);
            break;
        }

        if (firstVisibleColumn != i) {
            if (firstVisibleColumn > 0)
                resizeSection(logicalIndex(firstVisibleColumn), proxyModel->getColumnWidth(firstVisibleColumn) + 2 * kColumnPadding);

            firstVisibleColumn = i;
        }

        if (lastVisibleColumn != j) {
            if (lastVisibleColumn > 0)
                resizeSection(logicalIndex(lastVisibleColumn), proxyModel->getColumnWidth(lastVisibleColumn) + 2 * kColumnPadding);

            lastVisibleColumn = j;
        }
    }
}

void HeaderView::doFileNameColumnResize(const int totalWidth)
{
    int fileNameColumn = proxyModel()->getColumnByRole(kItemNameRole);
    int columnCount = count();
    int columnWidthSumOmitFileName = 0;

    for (int i = 0; i < columnCount; ++i) {
        if (i == fileNameColumn || isSectionHidden(i))
            continue;
        columnWidthSumOmitFileName += view->getColumnWidth(i);
    }

    int targetWidth = totalWidth - columnWidthSumOmitFileName;
    if (targetWidth >= minimumSectionSize()) {
        resizeSection(fileNameColumn, targetWidth);
    } else {
        resizeSection(fileNameColumn, minimumSectionSize());
    }
}

void HeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_EMIT mouseReleased();

    QHeaderView::mouseReleaseEvent(e);
}

void HeaderView::mouseMoveEvent(QMouseEvent *e)
{
    QHeaderView::mouseMoveEvent(e);

    int position = e->pos().x();
    int visual = visualIndexAt(position);
    if (visual == -1)
        return;
    int log = logicalIndex(visual);
    int pos = sectionViewportPosition(log);
    int grip = style()->pixelMetric(QStyle::PM_HeaderGripMargin, nullptr, this);

    bool atLeft = position < pos + grip;
    bool atRight = (position > pos + sectionSize(log) - grip);

    int result = -1;
    if (atLeft) {
        //grip at the beginning of the section
        while (visual > -1) {
            int logical = logicalIndex(--visual);
            if (!isSectionHidden(logical)) {
                result = logical;
                break;
            }
        }
    } else if (atRight) {
        //grip at the end of the section
        result = log;
    }

    if (result != -1) {
        if (!isChangeCursorState) {
            QApplication::setOverrideCursor(orientation() == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
            isChangeCursorState = true;
        }
    } else {
        if (isChangeCursorState) {
            QApplication::restoreOverrideCursor();
            isChangeCursorState = false;
        }
    }
}

void HeaderView::resizeEvent(QResizeEvent *e)
{
    Q_EMIT viewResized();

    QHeaderView::resizeEvent(e);
}

void HeaderView::leaveEvent(QEvent *e)
{
    if (isChangeCursorState) {
        QApplication::restoreOverrideCursor();
        isChangeCursorState = false;
    }

    QHeaderView::leaveEvent(e);
}

FileSortFilterProxyModel *HeaderView::proxyModel() const
{
    return qobject_cast<FileSortFilterProxyModel *>(model());
}
