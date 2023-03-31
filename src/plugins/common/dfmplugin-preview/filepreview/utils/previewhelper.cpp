// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmplugin_filepreview_global.h"
#include "previewhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/application/application.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

PreviewHelper *PreviewHelper::instance()
{
    static PreviewHelper instance;
    return &instance;
}

bool PreviewHelper::isPreviewEnabled()
{
    const auto &&ret = DConfigManager::instance()->value(ConfigInfos::kConfName, "previewEnable");
    return ret.isValid() ? ret.toBool() : true;
}

bool PreviewHelper::showThumbnailInRemote()
{
    const auto &&ret = DConfigManager::instance()->value(ConfigInfos::kConfName, ConfigInfos::kRemoteThumbnailKey, false);
    return ret.toBool();
}

bool PreviewHelper::showThumbnailInMtp()
{
    const auto &&ret = DConfigManager::instance()->value(ConfigInfos::kConfName, ConfigInfos::kMtpThumbnailKey, true);
    return ret.toBool();
}

void PreviewHelper::bindConfig()
{
    SyncPair pair {
        { SettingType::kGenAttr, Application::kShowThunmbnailInRemote },
        { ConfigInfos::kConfName, ConfigInfos::kRemoteThumbnailKey },
        saveRemoteToConf,
        syncRemoteToAppSet,
        isRemoteConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(pair);
}

PreviewHelper::PreviewHelper(QObject *parent)
    : QObject(parent)
{
}

void PreviewHelper::saveRemoteToConf(const QVariant &var)
{
    DConfigManager::instance()->setValue(ConfigInfos::kConfName, ConfigInfos::kRemoteThumbnailKey, var);
}

void PreviewHelper::syncRemoteToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setGenericAttribute(Application::kShowThunmbnailInRemote, var.toBool());
}

bool PreviewHelper::isRemoteConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toBool() && dset.toBool();
}
