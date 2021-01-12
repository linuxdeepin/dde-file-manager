/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
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
