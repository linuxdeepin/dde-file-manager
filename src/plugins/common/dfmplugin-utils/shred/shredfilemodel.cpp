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
    return createIndex(row, column, &files[row]);
}

QModelIndex ShredFileModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int ShredFileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return files.count();
}

int ShredFileModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ShredFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || files.count() <= index.row()) {
        fmWarning() << "ShredFileModel::data invalid index row:" << index.row() << "items count:" << files.count();
        return {};
    }

    if (role == Qt::DisplayRole)
        return files[index.row()].displayName;
    if (role == Qt::DecorationRole)
        return files[index.row()].icon;
    return {};
}

void ShredFileModel::setFileList(const QList<QUrl> &fileList)
{
    beginResetModel();
    files.clear();
    for (const auto &url : fileList) {
        auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoAuto);
        if (info && info->exists()) {
            files.append({url, info->displayOf(DisPlayInfoType::kFileDisplayName), info->fileIcon()});
        } else {
            fmWarning() << "The file is invalid: " << url;
        }
    }
    endResetModel();
}
