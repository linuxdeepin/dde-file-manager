/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dfmfactoryloader.h"
#include "private/dfmfactoryloader_p.h"

#include <QDir>
#include <QJsonArray>
#include <QPluginLoader>
#include <QDebug>

DFM_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QList<DFMFactoryLoader *>, qt_factory_loaders)

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, qt_factoryloader_mutex, (QMutex::Recursive))

QStringList DFMFactoryLoaderPrivate::pluginPaths;

DFMFactoryLoaderPrivate::DFMFactoryLoaderPrivate()
{
    if (pluginPaths.isEmpty()) {
        if (QT_PREPEND_NAMESPACE(qEnvironmentVariableIsEmpty)("DFM_PLUGIN_PATH"))
            pluginPaths.append(QString::fromLocal8Bit(PLUGINDIR).split(':'));
        else
            pluginPaths = QString::fromLocal8Bit(qgetenv("DFM_PLUGIN_PATH")).split(':');
    }
}

DFMFactoryLoaderPrivate::~DFMFactoryLoaderPrivate()
{

}

DFMFactoryLoader::DFMFactoryLoader(const char *iid,
                                   const QString &suffix,
                                   Qt::CaseSensitivity cs,
                                   bool repetitiveKeyInsensitive)
    : QObject(*new DFMFactoryLoaderPrivate)
{
    moveToThread(QCoreApplicationPrivate::mainThread());
    Q_D(DFMFactoryLoader);
    d->iid = iid;
    d->suffix = suffix;
    d->cs = cs;
    d->rki = repetitiveKeyInsensitive;

    QMutexLocker locker(qt_factoryloader_mutex());
    update();
    qt_factory_loaders()->append(this);
}

/* Internal, for debugging */
static bool dfm_debug_component()
{
#ifdef QT_DEBUG
    return true;
#endif

    static int debug_env = QT_PREPEND_NAMESPACE(qEnvironmentVariableIntValue)("DFM_DEBUG_PLUGINS");
    return debug_env != 0;
}

void DFMFactoryLoader::update()
{
    // Disable plugins on root user
    if (DFMGlobal::isRootUser()) {
        qWarning() << "Disable plugins for root user";

        return;
    }

#ifdef QT_SHARED
    Q_D(DFMFactoryLoader);
    qDebug() << "DFMFactoryLoader::DFMFactoryLoader() checking directory path";
    const QStringList &paths = d->pluginPaths;
    for (int i = 0; i < paths.count(); ++i) {
        const QString &pluginDir = paths.at(i);
        // Already loaded, skip it...
        if (d->loadedPaths.contains(pluginDir))
            continue;
        d->loadedPaths << pluginDir;

        QString path = pluginDir + d->suffix;

        if (dfm_debug_component())
            qDebug() << "DFMFactoryLoader::DFMFactoryLoader() checking directory path" << path << "...";

        if (!QDir(path).exists(QLatin1String(".")))
            continue;

        QStringList plugins = QDir(path).entryList(QDir::Files);
        QPluginLoader *loader = nullptr;

#ifdef Q_OS_MAC
        // Loading both the debug and release version of the cocoa plugins causes the objective-c runtime
        // to print "duplicate class definitions" warnings. Detect if DFMFactoryLoader is about to load both,
        // skip one of them (below).
        //
        // ### FIXME find a proper solution
        //
        const bool isLoadingDebugAndReleaseCocoa = plugins.contains(QStringLiteral("libqcocoa_debug.dylib"))
                                                   && plugins.contains(QStringLiteral("libqcocoa.dylib"));
#endif
        for (int j = 0; j < plugins.count(); ++j) {
            QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugins.at(j));

#ifdef Q_OS_MAC
            if (isLoadingDebugAndReleaseCocoa) {
#ifdef QT_DEBUG
                if (fileName.contains(QStringLiteral("libqcocoa.dylib")))
                    continue;    // Skip release plugin in debug mode
#else
                if (fileName.contains(QStringLiteral("libqcocoa_debug.dylib")))
                    continue;    // Skip debug plugin in release mode
#endif
            }
#endif
            if (dfm_debug_component()) {
                qDebug() << "DFMFactoryLoader::DFMFactoryLoader() looking at" << fileName;
            }
            loader = new QPluginLoader(fileName, this);
            if (!loader->load()) {
                if (dfm_debug_component()) {
                    qDebug() << loader->errorString();
                }
                loader->deleteLater();
                continue;
            }

            QStringList keys;
            bool metaDataOk = false;

            QString iid = loader->metaData().value(iidKeyLiteral()).toString();
            if (iid == QLatin1String(d->iid.constData(), d->iid.size())) {
                QJsonObject object = loader->metaData().value(metaDataKeyLiteral()).toObject();
                metaDataOk = true;

                QJsonArray k = object.value(keysKeyLiteral()).toArray();
                for (int m = 0; m < k.size(); ++m)
                    keys += d->cs ? k.at(m).toString() : k.at(m).toString().toLower();
            }
            if (dfm_debug_component())
                qDebug() << "Got keys from plugin meta data" << keys;


            if (!metaDataOk) {
                loader->deleteLater();
                continue;
            }

            int keyUsageCount = 0;
            for (int k = 0; k < keys.count(); ++k) {
                // first come first serve, unless the first
                // library was built with a future Qt version,
                // whereas the new one has a Qt version that fits
                // better
                const QString &key = keys.at(k);

                if (d->rki) {
                    d->keyMap.insertMulti(key, loader);
                    ++keyUsageCount;
                } else {
                    QPluginLoader *previous = d->keyMap.value(key);
                    int prev_dfm_version = 0;
                    if (previous) {
                        prev_dfm_version = (int)previous->metaData().value(versionKeyLiteral()).toDouble();
                    }
                    int dfm_version = (int)loader->metaData().value(versionKeyLiteral()).toDouble();
                    if (!previous || (prev_dfm_version > QString(QMAKE_VERSION).toDouble() && dfm_version <= QString(QMAKE_VERSION).toDouble())) {
                        d->keyMap.insertMulti(key, loader);
                        ++keyUsageCount;
                    }
                }
            }
            if (keyUsageCount || keys.isEmpty())
                d->pluginLoaderList += loader;
            else
                loader->deleteLater();
        }
    }
#else
    Q_D(DFMFactoryLoader);
    if (dfm_debug_component()) {
        qDebug() << "DFMFactoryLoader::DFMFactoryLoader() ignoring" << d->iid
                 << "since plugins are disabled in static builds";
    }
#endif
}

DFMFactoryLoader::~DFMFactoryLoader()
{
    Q_D(DFMFactoryLoader);

    QMutexLocker locker(qt_factoryloader_mutex());
    qt_factory_loaders()->removeAll(this);

    for (int i = 0; i < d->pluginLoaderList.count(); ++i) {
        QPluginLoader *loader = d->pluginLoaderList.at(i);
        loader->unload();
    }
}

QList<QJsonObject> DFMFactoryLoader::metaData() const
{
    Q_D(const DFMFactoryLoader);
    QMutexLocker locker(&d->mutex);
    QList<QJsonObject> metaData;
    for (int i = 0; i < d->pluginLoaderList.size(); ++i)
        metaData.append(d->pluginLoaderList.at(i)->metaData());

    return metaData;
}

QObject *DFMFactoryLoader::instance(int index) const
{
    Q_D(const DFMFactoryLoader);
    if (index < 0)
        return 0;

    if (index < d->pluginLoaderList.size()) {
        QPluginLoader *loader = d->pluginLoaderList.at(index);
        if (loader->instance()) {
            QObject *obj = loader->instance();
            if (obj) {
                if (!obj->parent())
                    obj->moveToThread(QCoreApplicationPrivate::mainThread());
                return obj;
            }
        }
        return 0;
    }

    return 0;
}

#if defined(Q_OS_UNIX) && !defined (Q_OS_MAC)
QPluginLoader *DFMFactoryLoader::pluginLoader(const QString &key) const
{
    Q_D(const DFMFactoryLoader);
    return d->keyMap.value(d->cs ? key : key.toLower());
}

QList<QPluginLoader *> DFMFactoryLoader::pluginLoaderList(const QString &key) const
{
    Q_D(const DFMFactoryLoader);
    return d->keyMap.values(d->cs ? key : key.toLower());
}
#endif

void DFMFactoryLoader::refreshAll()
{
    QMutexLocker locker(qt_factoryloader_mutex());
    QList<DFMFactoryLoader *> *loaders = qt_factory_loaders();
    for (QList<DFMFactoryLoader *>::const_iterator it = loaders->constBegin();
            it != loaders->constEnd(); ++it) {
        (*it)->update();
    }
}

QMultiMap<int, QString> DFMFactoryLoader::keyMap() const
{
    QMultiMap<int, QString> result;
    const QString metaDataKey = metaDataKeyLiteral();
    const QString keysKey = keysKeyLiteral();
    const QList<QJsonObject> metaDataList = metaData();
    for (int i = 0; i < metaDataList.size(); ++i) {
        const QJsonObject metaData = metaDataList.at(i).value(metaDataKey).toObject();
        const QJsonArray keys = metaData.value(keysKey).toArray();
        const int keyCount = keys.size();
        for (int k = 0; k < keyCount; ++k)
            result.insert(i, keys.at(k).toString());
    }
    return result;
}

int DFMFactoryLoader::indexOf(const QString &needle) const
{
    const QString metaDataKey = metaDataKeyLiteral();
    const QString keysKey = keysKeyLiteral();
    const QList<QJsonObject> metaDataList = metaData();
    for (int i = 0; i < metaDataList.size(); ++i) {
        const QJsonObject metaData = metaDataList.at(i).value(metaDataKey).toObject();
        const QJsonArray keys = metaData.value(keysKey).toArray();
        const int keyCount = keys.size();
        for (int k = 0; k < keyCount; ++k) {
            if (!keys.at(k).toString().compare(needle, Qt::CaseInsensitive))
                return i;
        }
    }
    return -1;
}

QList<int> DFMFactoryLoader::getAllIndexByKey(const QString &needle) const
{
    QList<int> list;

    const QString metaDataKey = metaDataKeyLiteral();
    const QString keysKey = keysKeyLiteral();
    const QList<QJsonObject> metaDataList = metaData();
    for (int i = 0; i < metaDataList.size(); ++i) {
        const QJsonObject metaData = metaDataList.at(i).value(metaDataKey).toObject();
        const QJsonArray keys = metaData.value(keysKey).toArray();
        const int keyCount = keys.size();
        for (int k = 0; k < keyCount; ++k) {
            if (!keys.at(k).toString().compare(needle, Qt::CaseInsensitive))
                list << i;
        }
    }
    return list;
}

DFM_END_NAMESPACE
