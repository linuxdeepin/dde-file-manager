// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHUTILS_H
#define TRASHUTILS_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QString>
#include <QStringList>
#include <QDateTime>

DFMBASE_BEGIN_NAMESPACE

namespace TrashUtils {

enum class TrashEmptyState {
    kUnknown,
    kEmpty,
    kNotEmpty
};

struct TrashItemInfo
{
    QUrl localFileUrl;   // resolved local file:// URL of the trashed file
    QUrl originalUrl;   // original location before deletion
    QDateTime deletionTime;   // when the file was trashed
};

bool trashIsEmpty();
TrashEmptyState trashEmptyState();
void setTrashEmptyState(TrashEmptyState state);
QUrl trashRootUrl();
bool isTrashFile(const QUrl &url);
bool isTrashRootFile(const QUrl &url);
bool fileCanTrash(const QUrl &url);
QString trashPathToNormal(const QString &trash);
QString normalPathToTrash(const QString &normal);

QStringList localTrashDirs();
TrashItemInfo resolveTrashUrl(const QUrl &url);
int countTrashItems();
qint64 calculateTrashSize();
QUrl localFileToTrashUrl(const QString &localPath);

}   // namespace TrashUtils

DFMBASE_END_NAMESPACE

#endif   // TRASHUTILS_H
