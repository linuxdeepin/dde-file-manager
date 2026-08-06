// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHREDFILEMODEL_H
#define SHREDFILEMODEL_H

#include "dfmplugin_utils_global.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QUrl>

namespace dfmplugin_utils {

class ShredFileModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ShredFileModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setFileList(const QList<QUrl> &fileList);

private:
    struct CachedFile {
        QUrl url;
        QString displayName;
        QIcon icon;
    };
    QList<CachedFile> files;
};
}

#endif   // SHREDFILEMODEL_H
