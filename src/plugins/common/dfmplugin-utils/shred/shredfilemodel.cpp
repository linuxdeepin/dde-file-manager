// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shredfilemodel.h"

#include <dfm-base/base/schemefactory.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

ShredFileModel::ShredFileModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex ShredFileModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (row >= rowCount() || row < 0)
        return QModelIndex();
    return createIndex(row, column, &urlList[row]);
}

QModelIndex ShredFileModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int ShredFileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return urlList.count();
}

int ShredFileModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ShredFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || urlList.count() <= index.row()) {
        fmWarning() << "ComputerModel::data invalid index row:" << index.row() << "items count:" << urlList.count();
        return {};
    }

    const auto &url = urlList[index.row()];
    auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoAuto);
    if (!info || !info->exists()) {
        fmWarning() << "The file is invalid: " << url;
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return info->displayOf(DisPlayInfoType::kFileDisplayName);
    case Qt::DecorationRole:
        return info->fileIcon();
    default:
        return {};
    }
}

void ShredFileModel::setFileList(const QList<QUrl> &fileList)
{
    beginResetModel();
    urlList = fileList;
    endResetModel();
}
