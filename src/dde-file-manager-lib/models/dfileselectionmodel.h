// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILESELECTIONMODEL_H
#define DFILESELECTIONMODEL_H

#include <QItemSelectionModel>
#include <QTimer>
class DFileView;
namespace FileManagerSelectionModel {
class DFileSelectionModel : public QItemSelectionModel
{
    friend class ::DFileView;
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
};
}
#endif // DFILESELECTIONMODEL_H
