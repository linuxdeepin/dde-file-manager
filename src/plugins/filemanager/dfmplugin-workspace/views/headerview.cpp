// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "headerview.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QApplication>
#include <QMouseEvent>
#include <QMenu>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

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
    auto model = viewModel();
    if (model) {
        int columnCount = count();
        int i = 0;
        int j = columnCount - 1;

        for (; i < columnCount; ++i) {
            int logicalIndex = this->logicalIndex(i);
            if (isSectionHidden(logicalIndex))
                continue;

            resizeSection(logicalIndex, model->getColumnWidth(i) + kLeftPadding + kListModeLeftMargin + 2 * kColumnPadding);
            break;
        }

        for (; j > 0; --j) {
            int logicalIndex = this->logicalIndex(j);
            if (isSectionHidden(logicalIndex))
                continue;

            resizeSection(logicalIndex, model->getColumnWidth(j) + kRightPadding + kListModeRightMargin + 2 * kColumnPadding);
            break;
        }

        if (firstVisibleColumn != i) {
            if (firstVisibleColumn > 0)
                resizeSection(logicalIndex(firstVisibleColumn), model->getColumnWidth(firstVisibleColumn) + 2 * kColumnPadding);

            firstVisibleColumn = i;
        }

        if (lastVisibleColumn != j) {
            if (lastVisibleColumn > 0)
                resizeSection(logicalIndex(lastVisibleColumn), model->getColumnWidth(lastVisibleColumn) + 2 * kColumnPadding);

            lastVisibleColumn = j;
        }
    }
}

void HeaderView::doFileNameColumnResize(const int totalWidth)
{
    int fileNameColumn = viewModel()->getColumnByRole(kItemFileDisplayNameRole);
    int columnCount = count();
    int columnWidthSumOmitFileName = 0;

    for (int i = 0; i < columnCount; ++i) {
        if (i == fileNameColumn || isSectionHidden(i))
            continue;
        columnWidthSumOmitFileName += view->getColumnWidth(i);
    }

    int targetWidth = totalWidth - columnWidthSumOmitFileName;
    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();
    int colWidth = state.value(QString::number(kItemFileDisplayNameRole), -1).toInt();

    resizeSection(fileNameColumn, qMax(targetWidth, colWidth));
}

void HeaderView::onActionClicked(const int column, QAction *action)
{
    action->setChecked(!action->isChecked());
    setSectionHidden(column, action->isChecked());

    emit hiddenSectionChanged(action->text(), action->isChecked());
}

void HeaderView::mousePressEvent(QMouseEvent *e)
{
    Q_EMIT mousePressed();

    QHeaderView::mousePressEvent(e);
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

void HeaderView::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)

    QMenu *menu = new QMenu;
    auto model = viewModel();

    for (int i = 0; i < count(); ++i) {
        // file name can not hidden
        int role = model->getRoleByColumn(i);
        if (role == kItemNameRole || role == kItemFileDisplayNameRole)
            continue;

        QAction *action = new QAction(menu);

        action->setText(model->roleDisplayString(role));
        action->setCheckable(true);
        action->setChecked(!isSectionHidden(i));

        connect(action, &QAction::triggered, this, [=] {
            onActionClicked(i, action);
        });

        menu->addAction(action);
    }

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void HeaderView::paintEvent(QPaintEvent *e)
{
    QHeaderView::paintEvent(e);
    int idealHeight = qMax(kListViewHeaderHeight, fontMetrics().height());
    if (idealHeight != this->height())
        this->setFixedHeight(idealHeight);
}

FileViewModel *HeaderView::viewModel() const
{
    return qobject_cast<FileViewModel *>(model());
}
