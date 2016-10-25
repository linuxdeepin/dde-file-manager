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
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;
    void clear() Q_DECL_OVERRIDE;

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
