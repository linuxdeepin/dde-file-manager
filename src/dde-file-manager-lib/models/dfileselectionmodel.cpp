// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfileselectionmodel.h"

#include <QDebug>
using namespace FileManagerSelectionModel;
DFileSelectionModel::DFileSelectionModel(QAbstractItemModel *model)
    : QItemSelectionModel(model)
{
    m_timer.setSingleShot(true);

    connect(&m_timer, &QTimer::timeout, this, &DFileSelectionModel::updateSelecteds);
}

DFileSelectionModel::DFileSelectionModel(QAbstractItemModel *model, QObject *parent)
    : QItemSelectionModel(model, parent)
{
    m_timer.setSingleShot(true);

    connect(&m_timer, &QTimer::timeout, this, &DFileSelectionModel::updateSelecteds);
}

bool DFileSelectionModel::isSelected(const QModelIndex &index) const
{
    if (m_currentCommand != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect))
        return QItemSelectionModel::isSelected(index);

    auto ret = std::any_of(m_selection.begin(), m_selection.end(), [index](const QItemSelectionRange & range) {
        return range.contains(index);
    });

    if (ret) {
        Qt::ItemFlags flags = index.flags();
        return (flags & Qt::ItemIsSelectable);
    }

    return false;
}

int DFileSelectionModel::selectedCount() const
{
    if (m_currentCommand != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect))
        return selectedIndexes().count();

    return m_lastSelectedIndex.isValid() ? (m_lastSelectedIndex.row() - m_firstSelectedIndex.row() + 1) : 0;
}

QModelIndexList DFileSelectionModel::selectedIndexes() const
{
    if (m_selectedList.isEmpty()) {
        if (m_currentCommand != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect)) {
            m_selectedList = QItemSelectionModel::selectedIndexes();
        } else {
            for (const QItemSelectionRange &range : m_selection) {
                m_selectedList << range.indexes();
            }
        }
    }

    return m_selectedList;
}

void DFileSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    if (!command.testFlag(NoUpdate))
        m_selectedList.clear();

    if (command != QItemSelectionModel::SelectionFlags(Current | Rows | ClearAndSelect)) {
        if (m_timer.isActive()) {
            m_timer.stop();
            updateSelecteds();
        }

        m_currentCommand = command;

        QItemSelectionModel::select(selection, command);

        return;
    }


    if (selection.isEmpty()) {
        m_firstSelectedIndex = QModelIndex();
        m_lastSelectedIndex = QModelIndex();
    } else {
        m_firstSelectedIndex = selection.first().topLeft();
        m_lastSelectedIndex = selection.last().bottomRight();
    }

    QItemSelection newSelection(m_firstSelectedIndex, m_lastSelectedIndex);

    emitSelectionChanged(newSelection, m_selection);

    m_currentCommand = command;
    m_selection = newSelection;

    m_timer.start(20);
}

void DFileSelectionModel::clear()
{
    m_timer.stop();
    m_selectedList.clear();
    m_selection.clear();

    QItemSelectionModel::clear();
}

void DFileSelectionModel::updateSelecteds()
{
    QItemSelectionModel::select(m_selection, m_currentCommand);
}
