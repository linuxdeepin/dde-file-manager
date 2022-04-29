#ifndef COLLECTIONDELEGATE_H
#define COLLECTIONDELEGATE_H

#include <QStyledItemDelegate>

class CollectionDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CollectionDelegate(QObject *parent = nullptr);
    virtual ~CollectionDelegate()override;
    virtual QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
signals:
    void removeItemManually(const QString& text,int row);
protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif // COLLECTIONDELEGATE_H
