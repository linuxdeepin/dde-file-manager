/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMFACTORYLOADER_H
#define DFMFACTORYLOADER_H

#include "dfmglobal.h"

#include <QMap>
#include <QPluginLoader>

DFM_BEGIN_NAMESPACE

class DFMFactoryLoaderPrivate;
class DFMFactoryLoader : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMFactoryLoader)
public:
    explicit DFMFactoryLoader(const char *iid,
                              const QString &suffix = QString(),
                              Qt::CaseSensitivity = Qt::CaseSensitive,
                              bool repetitiveKeyInsensitive  = false);
    ~DFMFactoryLoader();

    QList<QJsonObject> metaData() const;
    QObject *instance(int index) const;

#if defined(Q_OS_UNIX) && !defined (Q_OS_MAC)
    QPluginLoader *pluginLoader(const QString &key) const;
    QList<QPluginLoader*> pluginLoaderList(const QString &key) const;
#endif

    QMultiMap<int, QString> keyMap() const;
    int indexOf(const QString &needle) const;
    QList<int> getAllIndexByKey(const QString &needle) const;

    void update();

    static void refreshAll();
};

template <class PluginInterface, class FactoryInterface>
    PluginInterface *dLoadPlugin(const DFMFactoryLoader *loader, const QString &key)
{
    const int index = loader->indexOf(key);
    if (index != -1) {
        QObject *factoryObject = loader->instance(index);
        if (FactoryInterface *factory = qobject_cast<FactoryInterface *>(factoryObject))
            if (PluginInterface *result = factory->create(key))
                return result;
    }
    return 0;
}

template <class PluginInterface, class FactoryInterface>
    QList<PluginInterface*> dLoadPluginList(const DFMFactoryLoader *loader, const QString &key)
{
    QList<PluginInterface*> list;

    for (int index : loader->getAllIndexByKey(key)) {
        if (index != -1) {
            QObject *factoryObject = loader->instance(index);
            if (FactoryInterface *factory = qobject_cast<FactoryInterface *>(factoryObject))
                if (PluginInterface *result = factory->create(key))
                    list << result;
        }
    }

    return list;
}

template <class PluginInterface, class FactoryInterface, class Parameter1>
PluginInterface *dLoadPlugin(const DFMFactoryLoader *loader,
                              const QString &key,
                              const Parameter1 &parameter1)
{
    const int index = loader->indexOf(key);
    if (index != -1) {
        QObject *factoryObject = loader->instance(index);
        if (FactoryInterface *factory = qobject_cast<FactoryInterface *>(factoryObject))
            if (PluginInterface *result = factory->create(key, parameter1))
                return result;
    }
    return 0;
}

DFM_END_NAMESPACE

#endif // DFMFACTORYLOADER_H
