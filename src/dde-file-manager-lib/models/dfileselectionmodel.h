/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef DFILESELECTIONMODEL_H
#define DFILESELECTIONMODEL_H

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
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
    void clear() override;

private:
    void updateSelecteds();

    mutable QModelIndexList m_selectedList;

    QItemSelection m_selection;
    QModelIndex m_firstSelectedIndex;
    QModelIndex m_lastSelectedIndex;
    QItemSelectionModel::SelectionFlags m_currentCommand;
    QTimer m_timer;

    friend class DFileView;
};

#endif // DFILESELECTIONMODEL_H
