// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspaceconfighelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/application/application.h>

using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::ViewConfig;
DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void WorkspaceConfigHelper::initConfig() {
    SyncPair thumbnailPair {
        { SettingType::kGenAttr, Application::kShowThunmbnailInRemote },
        { DConfigInfo::kConfName, DConfigInfo::kRemoteThumbnailKey },
        saveRemoteThumbnailToConf,
        syncRemoteThumbnailToAppSet,
        isRemoteThumbnailConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(thumbnailPair);

    SyncPair iconSizePair {
        { SettingType::kAppAttr, Application::kIconSizeLevel },
        { kViewDConfName, kIconSizeLevel },
        saveIconSizeToConf,
        syncIconSizeToAppSet,
        isIconSizeConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(iconSizePair);

    SyncPair gridDensityPair {
        { SettingType::kAppAttr, Application::kGridDensityLevel },
        { kViewDConfName, kIconGridDensityLevel },
        saveGridDensityToConf,
        syncGridDensityToAppSet,
        isGridDensityConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(gridDensityPair);

    SyncPair listHeightPair {
        { SettingType::kAppAttr, Application::kListHeightLevel },
        { kViewDConfName, kListHeightLevel },
        saveListHeightToConf,
        syncListHeightToAppSet,
        isListHeightConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(listHeightPair);
}

void WorkspaceConfigHelper::saveRemoteThumbnailToConf(const QVariant &var)
{
    DConfigManager::instance()->setValue(DConfigInfo::kConfName, DConfigInfo::kRemoteThumbnailKey, var);
}

void WorkspaceConfigHelper::syncRemoteThumbnailToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setGenericAttribute(Application::kShowThunmbnailInRemote, var.toBool());
}

bool WorkspaceConfigHelper::isRemoteThumbnailConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toBool() && dset.toBool();
}

void WorkspaceConfigHelper::saveIconSizeToConf(const QVariant &var)
{
    DConfigManager::instance()->setValue(kViewDConfName, kIconSizeLevel, var);
}

void WorkspaceConfigHelper::syncIconSizeToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setAppAttribute(Application::kIconSizeLevel, var.toInt());
}

bool WorkspaceConfigHelper::isIconSizeConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toInt() == dset.toInt();
}

void WorkspaceConfigHelper::saveGridDensityToConf(const QVariant &var)
{
    DConfigManager::instance()->setValue(kViewDConfName, kIconGridDensityLevel, var);
}

void WorkspaceConfigHelper::syncGridDensityToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setAppAttribute(Application::kGridDensityLevel, var.toInt());
}

bool WorkspaceConfigHelper::isGridDensityConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toInt() == dset.toInt();
}

void WorkspaceConfigHelper::saveListHeightToConf(const QVariant &var)
{
    DConfigManager::instance()->setValue(kViewDConfName, kListHeightLevel, var);
}

void WorkspaceConfigHelper::syncListHeightToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setAppAttribute(Application::kListHeightLevel, var.toInt());
}

bool WorkspaceConfigHelper::isListHeightConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toInt() == dset.toInt();
}
