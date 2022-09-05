// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmgenericfactory.h"
#include "dfmgenericplugin.h"
#include "dfmfactoryloader.h"

DFM_BEGIN_NAMESPACE

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(DFMFactoryLoader, loader,
    (DFMGenericFactoryInterface_iid, QLatin1String("/generics"), Qt::CaseInsensitive, true))
#endif

DFMGenericFactory::DFMGenericFactory()
{

}

QStringList DFMGenericFactory::keys()
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

QObject *DFMGenericFactory::create(const QString &key)
{
    return dLoadPlugin<QObject, DFMGenericPlugin>(loader(), key);
}

QObjectList DFMGenericFactory::createAll(const QString &key)
{
    return dLoadPluginList<QObject, DFMGenericPlugin>(loader(), key);
}

DFM_END_NAMESPACE
