#include "ddirmodel.h"
#include <QDebug>

DDirModel::DDirModel(QCompleter *completer)
    : QStringListModel(completer)
{
    m_dirModel = new QDirModel;
}

QVariant DDirModel::data(const QModelIndex &index, int role) const
{
    qDebug() << index.row() << index.column() << role;
    return m_dirModel->data(index, role);
}

int DDirModel::rowCount(const QModelIndex &/*parent*/) const
{
    return QStringListModel::rowCount();
}
