// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "servicemanager.h"
#include "vaulthelper.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/application/settings.h>

#include <mutex>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

ServiceManager::ServiceManager(QObject *parent)
    : QObject(parent)
{
}

ServiceManager::ExpandFieldMap ServiceManager::basicViewFieldFunc(const QUrl &url)
{
    BasicExpand expandFiledMap;
    expandFiledMap.insert(kFilePosition, qMakePair(tr("Location"), url.url()));

    ExpandFieldMap expandMap;
    expandMap.insert(kFieldReplace, expandFiledMap);
    return expandMap;
}

ServiceManager::ExpandFieldMap ServiceManager::detailViewFieldFunc(const QUrl &url)
{
    BasicExpand expandFiledMap;
    ExpandFieldMap expandMap;
    Settings setting(kVaultTimeConfigFile);
    QString lockedTime;
    if (setting.value(kjsonGroupName, kjsonKeyLockTime).toString().isEmpty())
        lockedTime = setting.value(kjsonGroupName, kjsonKeyInterviewItme).toString();
    else
        lockedTime = setting.value(kjsonGroupName, kjsonKeyLockTime).toString();

    if (!UniversalUtils::urlEquals(url, VaultHelper::instance()->rootUrl()))
        return expandMap;
    expandFiledMap.insert(kFileInterviewTime, qMakePair(tr("Time locked"), lockedTime));

    expandMap.insert(kFieldInsert, expandFiledMap);
    return expandMap;
}
