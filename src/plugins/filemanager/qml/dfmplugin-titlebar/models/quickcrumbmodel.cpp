// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickcrumbmodel.h"

namespace dfmplugin_titlebar {

QuickCrumbModel::QuickCrumbModel(QObject *parent)
    : QAbstractListModel { parent }
{
}

void QuickCrumbModel::setCurrentUrl(const QUrl &url, const QList<CrumbData> &crumbDataList)
{
    beginResetModel();
    fileUrl = url;
    crumbData = crumbDataList;
    endResetModel();
}

int QuickCrumbModel::rowCount(const QModelIndex &) const
{
    return crumbData.size();
}

QVariant QuickCrumbModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= crumbData.size()) {
        return {};
    }

    const CrumbData &data = crumbData.at(index.row());
    switch (role) {
    case FileUrlRole:
        return data.url;
    case FullUrlRole:
        return fileUrl;
    case TextRole:
        return data.displayText;
    case IconRole:
        return data.iconName;
    case UseIconRole:
        return !data.iconName.isEmpty();
    }

    return {};
}

QHash<int, QByteArray> QuickCrumbModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FileUrlRole] = "fileUrl";
    roles[FullUrlRole] = "fullUrl";
    roles[TextRole] = "text";
    roles[IconRole] = "icon";
    roles[UseIconRole] = "useIcon";
    return roles;
}

}
