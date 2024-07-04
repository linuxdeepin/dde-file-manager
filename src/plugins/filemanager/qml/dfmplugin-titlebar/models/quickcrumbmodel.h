// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUICKCRUMBMODEL_H
#define QUICKCRUMBMODEL_H

#include "dfmplugin_titlebar_global.h"

#include <QAbstractListModel>
#include <QUrl>

namespace dfmplugin_titlebar {

class QuickCrumbModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        FileUrlRole = Qt::UserRole + 1,
        FullUrlRole,
        TextRole,
        IconRole,
        UseIconRole,
    };
    Q_ENUM(Roles)

    explicit QuickCrumbModel(QObject *parent = nullptr);

    void setCurrentUrl(const QUrl &url, const QList<CrumbData> &crumbDataList);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QUrl fileUrl;
    QList<CrumbData> crumbData;
};

}

#endif   // QUICKCRUMBMODEL_H
