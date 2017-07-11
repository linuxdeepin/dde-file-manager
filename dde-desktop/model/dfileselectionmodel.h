/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QItemSelectionModel>
#include <QTimer>

class DFileSelectionModel : public QItemSelectionModel
{
public:
    explicit DFileSelectionModel(QAbstractItemModel *model = 0);
    explicit DFileSelectionModel(QAbstractItemModel *model, QObject *parent);

    bool isSelected(const QModelIndex &index) const;
    int selectedCount() const;

    QModelIndexList selectedIndexes() const;

protected:
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;

private:
    void updateSelecteds();

    mutable QModelIndexList m_selectedList;

    QItemSelection m_selection;
    QModelIndex m_firstSelectedIndex;
    QModelIndex m_lastSelectedIndex;
    QItemSelectionModel::SelectionFlags m_currentCommand;
    QTimer m_timer;

    friend class CanvasGridView;
};
