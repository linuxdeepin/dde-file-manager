// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACECONFIGHELPER_H
#define WORKSPACECONFIGHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QVariant>
#include <QString>

namespace dfmplugin_workspace {

class WorkspaceConfigHelper
{
public:
    static void initConfig();

    static void saveRemoteThumbnailToConf(const QVariant &var);
    static void syncRemoteThumbnailToAppSet(const QString &, const QString &, const QVariant &var);
    static bool isRemoteThumbnailConfEqual(const QVariant &dcon, const QVariant &dset);

    static void saveIconSizeToConf(const QVariant &var);
    static void syncIconSizeToAppSet(const QString &, const QString &, const QVariant &var);
    static bool isIconSizeConfEqual(const QVariant &dcon, const QVariant &dset);

    static void saveGridDensityToConf(const QVariant &var);
    static void syncGridDensityToAppSet(const QString &, const QString &, const QVariant &var);
    static bool isGridDensityConfEqual(const QVariant &dcon, const QVariant &dset);

    static void saveListHeightToConf(const QVariant &var);
    static void syncListHeightToAppSet(const QString &, const QString &, const QVariant &var);
    static bool isListHeightConfEqual(const QVariant &dcon, const QVariant &dset);
};

}   // namespace dfmplugin_workspace

#endif // WORKSPACECONFIGHELPER_H
