// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "dfmplugin_filepreview_global.h"

#include <QObject>
#include <QMutex>
#include <QMultiMap>
#include <QPluginLoader>

namespace dfmplugin_filepreview {
class PreviewPluginLoaderPrivate;
class PreviewPluginLoader : public QObject
{
    Q_OBJECT
public:
    explicit PreviewPluginLoader(const char *iid,
                                 const QString &suffix = QString(),
                                 Qt::CaseSensitivity = Qt::CaseSensitive,
                                 bool repetitiveKeyInsensitive = false);
    virtual ~PreviewPluginLoader();

    QList<QJsonObject> metaData() const;
    QObject *instance(int index) const;

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    QPluginLoader *pluginLoader(const QString &key) const;
    QList<QPluginLoader *> pluginLoaderList(const QString &key) const;
#endif

    QMultiMap<int, QString> keyMap() const;
    int indexOf(const QString &needle) const;
    QList<int> getAllIndexByKey(const QString &needle) const;

    void update();

    static void refreshAll();

    PreviewPluginLoaderPrivate *dptr { nullptr };
};

template<class PluginInterface, class FactoryInterface>
PluginInterface *dLoadPlugin(const PreviewPluginLoader *loader, const QString &key)
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

template<class PluginInterface, class FactoryInterface>
QList<PluginInterface *> dLoadPluginList(const PreviewPluginLoader *loader, const QString &key)
{
    QList<PluginInterface *> list;

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
}
#endif
