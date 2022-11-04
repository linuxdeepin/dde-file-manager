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
#include "fileselectionmodel.h"
#include "private/fileselectionmodel_p.h"

using namespace dfmplugin_workspace;

FileSelectionModel::FileSelectionModel(QAbstractItemModel *model)
    : QItemSelectionModel(model),
      d(new FileSelectionModelPrivate(this))
{
}

FileSelectionModel::FileSelectionModel(QAbstractItemModel *model, QObject *parent)
    : QItemSelectionModel(model, parent),
      d(new FileSelectionModelPrivate(this))
{
}

FileSelectionModel::~FileSelectionModel()
{
}

bool FileSelectionModel::isSelected(const QModelIndex &index) const
{
    if (d->currentCommand != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect))
        return QItemSelectionModel::isSelected(index);

    auto ret = std::any_of(d->selection.begin(), d->selection.end(), [index](const QItemSelectionRange &range) {
        return range.contains(index);
    });

    if (ret) {
        Qt::ItemFlags flags = index.flags();
        return (flags & Qt::ItemIsSelectable);
    }

    return false;
}

int FileSelectionModel::selectedCount() const
{
    if (d->currentCommand != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect))
        return selectedIndexes().count();

    bool selectionValid = d->firstSelectedIndex.isValid() && d->lastSelectedIndex.isValid();
    return selectionValid ? (d->lastSelectedIndex.row() - d->firstSelectedIndex.row() + 1) : 0;
}

QModelIndexList FileSelectionModel::selectedIndexes() const
{
    if (d->selectedList.isEmpty()) {
        if (d->currentCommand != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect)) {
            d->selectedList = QItemSelectionModel::selectedIndexes();
        } else {
            for (const QItemSelectionRange &range : d->selection) {
                d->selectedList << range.indexes();
            }
        }

        auto isInVaildIndex = [=](const QModelIndex &index) {
            return index.column() != 0;
        };

        d->selectedList.erase(std::remove_if(d->selectedList.begin(), d->selectedList.end(), isInVaildIndex),
                              d->selectedList.end());
    }
    return d->selectedList;
}

void FileSelectionModel::updateSelecteds()
{
    QItemSelectionModel::select(d->selection, d->currentCommand);
}

void FileSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    if (!command.testFlag(NoUpdate))
        d->selectedList.clear();

    if (command != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect)) {
        if (d->timer.isActive()) {
            d->timer.stop();
            updateSelecteds();
        }

        d->currentCommand = command;

        QItemSelectionModel::select(selection, command);

        return;
    }

    if (selection.isEmpty()) {
        d->firstSelectedIndex = QModelIndex();
        d->lastSelectedIndex = QModelIndex();
    } else {
        d->firstSelectedIndex = selection.first().topLeft();
        d->lastSelectedIndex = selection.last().bottomRight();
    }

    QItemSelection newSelection(d->firstSelectedIndex, d->lastSelectedIndex);

    emitSelectionChanged(newSelection, d->selection);

    d->currentCommand = command;
    d->selection = newSelection;

    d->timer.start(20);
}

void FileSelectionModel::clear()
{
    d->timer.stop();
    d->selectedList.clear();
    d->selection.clear();
    d->firstSelectedIndex = QModelIndex();
    d->lastSelectedIndex = QModelIndex();

    QItemSelectionModel::clear();
}
