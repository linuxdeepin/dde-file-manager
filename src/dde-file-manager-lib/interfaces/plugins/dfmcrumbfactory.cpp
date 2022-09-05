// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmcrumbfactory.h"
#include "dfmcrumbplugin.h"
#include "dfmfactoryloader.h"

DFM_BEGIN_NAMESPACE

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(DFMFactoryLoader, loader,
    (DFMCrumbFactoryInterface_iid, QLatin1String("/crumbs"), Qt::CaseInsensitive))
#endif

QStringList DFMCrumbFactory::keys()
{
    QStringList list;
#ifndef QT_NO_LIBRARY
    typedef QMultiMap<int, QString> PluginKeyMap;

    const PluginKeyMap keyMap = loader()->keyMap();
    const PluginKeyMap::const_iterator cend = keyMap.constEnd();
    for (PluginKeyMap::const_iterator it = keyMap.constBegin(); it != cend; ++it)
        list.append(it.value());
#endif
    return list;
}

DFMCrumbInterface *DFMCrumbFactory::create(const QString &key)
{
    return dLoadPlugin<DFMCrumbInterface, DFMCrumbPlugin>(loader(), key);
}

DFM_END_NAMESPACE
