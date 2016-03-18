#ifndef DDIRMODEL_H
#define DDIRMODEL_H

#include <QStringListModel>
#include <QDirModel>
#include <QCompleter>

class DDirModel : public QStringListModel
{
public:
    DDirModel(QCompleter * completer);
    QVariant data(const QModelIndex &index, int role) const;
private:
    QDirModel * m_dirModel;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const;
};

#endif // DDIRMODEL_H
