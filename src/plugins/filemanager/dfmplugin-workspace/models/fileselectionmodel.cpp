// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

void FileSelectionModel::clearSelectList()
{
    d->selectedList.clear();
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

    // 这里不能发送 emitSelectionChanged这个信号
    // 发送了这个信号当timer超时去执行QItemSelectionModel::select时，select相当没有改变，不会再次发送emitSelectionChanged
    // QItemSelectionModel::selectedIndexes()会返回以前的，不会是当前的newSelection；

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
