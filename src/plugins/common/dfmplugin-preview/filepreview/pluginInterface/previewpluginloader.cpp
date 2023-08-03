// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewpluginloader.h"
#include "private/pluginloader_p.h"

#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>

using namespace dfmplugin_filepreview;
Q_GLOBAL_STATIC(QList<PreviewPluginLoader *>, qt_factory_loaders)

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, qt_factoryloader_mutex, (QMutex::Recursive))

/* Internal, for debugging */
static bool dfm_debug_component()
{
#ifdef QT_DEBUG
    return true;
#endif

    static int debug_env = QT_PREPEND_NAMESPACE(qEnvironmentVariableIntValue)("DFM_DEBUG_PLUGINS");
    return debug_env != 0;
}

PreviewPluginLoader::PreviewPluginLoader(const char *iid, const QString &suffix, Qt::CaseSensitivity cs, bool repetitiveKeyInsensitive)
    : QObject(nullptr), dptr(new PreviewPluginLoaderPrivate)
{
    dptr->iid = iid;
    dptr->suffix = suffix;
    dptr->cs = cs;
    dptr->rki = repetitiveKeyInsensitive;
    QMutexLocker locker(qt_factoryloader_mutex());
    update();
    qt_factory_loaders->append(this);
}

PreviewPluginLoader::~PreviewPluginLoader()
{
    QMutexLocker locker(qt_factoryloader_mutex());
    qt_factory_loaders()->removeAll(this);

    for (int i = 0; i < dptr->pluginLoaderList.count(); ++i) {
        QPluginLoader *loader = dptr->pluginLoaderList.at(i);
        loader->unload();
    }
}

QList<QJsonObject> PreviewPluginLoader::metaData() const
{
    QMutexLocker locker(&dptr->mutex);
    QList<QJsonObject> metaData;
    for (int i = 0; i < dptr->pluginLoaderList.size(); ++i)
        metaData.append(dptr->pluginLoaderList.at(i)->metaData());

    return metaData;
}

QObject *PreviewPluginLoader::instance(int index) const
{
    if (index < 0)
        return nullptr;

    if (index < dptr->pluginLoaderList.size()) {
        QPluginLoader *loader = dptr->pluginLoaderList.at(index);
        if (loader->instance()) {
            QObject *obj = loader->instance();
            if (obj) {
                if (!obj->parent())
                    obj->moveToThread(qApp->thread());
                return obj;
            }
        }
        return nullptr;
    }

    return nullptr;
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
QPluginLoader *PreviewPluginLoader::pluginLoader(const QString &key) const
{
    return dptr->keyMap.value(dptr->cs ? key : key.toLower());
}

QList<QPluginLoader *> PreviewPluginLoader::pluginLoaderList(const QString &key) const
{
    return dptr->keyMap.values(dptr->cs ? key : key.toLower());
}
#endif

QMultiMap<int, QString> PreviewPluginLoader::keyMap() const
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

int PreviewPluginLoader::indexOf(const QString &needle) const
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

QList<int> PreviewPluginLoader::getAllIndexByKey(const QString &needle) const
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

void PreviewPluginLoader::update()
{

#ifdef QT_SHARED
    qInfo() << "File Preview: checking directory path";
    const QStringList &paths = dptr->pluginPaths;
    for (int i = 0; i < paths.count(); ++i) {
        const QString &pluginDir = paths.at(i);
        //! Already loaded, skip it...
        if (!dptr->loadedPaths.contains(pluginDir))
            dptr->loadedPaths << pluginDir;

        QString path = pluginDir;

        qInfo() << "File Preview: checking directory path" << path << "...";

        if (!QDir(path).exists(QLatin1String(".")))
            continue;

        QStringList plugins = QDir(path).entryList(QDir::Files);
        QPluginLoader *loader = nullptr;

#    ifdef Q_OS_MAC
        // Loading both the debug and release version of the cocoa plugins causes the objective-c runtime
        // to print "duplicate class definitions" warnings. Detect if DFMFactoryLoader is about to load both,
        // skip one of them (below).
        //
        // ### FIXME find a proper solution
        //
        const bool isLoadingDebugAndReleaseCocoa = plugins.contains(QStringLiteral("libqcocoa_debug.dylib"))
                && plugins.contains(QStringLiteral("libqcocoa.dylib"));
#    endif
        for (int j = 0; j < plugins.count(); ++j) {
            QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugins.at(j));

#    ifdef Q_OS_MAC
            if (isLoadingDebugAndReleaseCocoa) {
#        ifdef QT_DEBUG
                if (fileName.contains(QStringLiteral("libqcocoa.dylib")))
                    continue;   // Skip release plugin in debug mode
#        else
                if (fileName.contains(QStringLiteral("libqcocoa_debug.dylib")))
                    continue;   // Skip debug plugin in release mode
#        endif
            }
#    endif
            if (dfm_debug_component()) {
                qInfo() << "File Preview: looking at" << fileName;
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
            if (iid == QLatin1String(dptr->iid.constData(), dptr->iid.size())) {
                QJsonObject object = loader->metaData().value(metaDataKeyLiteral()).toObject();
                metaDataOk = true;

                QJsonArray k = object.value(keysKeyLiteral()).toArray();
                for (int m = 0; m < k.size(); ++m)
                    keys += dptr->cs ? k.at(m).toString() : k.at(m).toString().toLower();
            }
            if (dfm_debug_component())
                qInfo() << "File Preview: Got keys from plugin meta data" << keys;

            if (!metaDataOk) {
                loader->deleteLater();
                continue;
            }

            int keyUsageCount = 0;
            for (int k = 0; k < keys.count(); ++k) {
                /*! first come first serve, unless the first
                 *library was built with a future Qt version,
                 * whereas the new one has a Qt version that fits
                 * better
                */
                const QString &key = keys.at(k);

                if (dptr->rki) {
                    dptr->keyMap.insertMulti(key, loader);
                    ++keyUsageCount;
                } else {
                    QPluginLoader *previous = dptr->keyMap.value(key);
                    int prev_dfm_version = 0;
                    if (previous) {
                        prev_dfm_version = static_cast<int>(previous->metaData().value(versionKeyLiteral()).toDouble());
                    }
                    int dfm_version = static_cast<int>(loader->metaData().value(versionKeyLiteral()).toDouble());
                    if (!previous || (prev_dfm_version > QString(VERSION).toDouble() && dfm_version <= QString(VERSION).toDouble())) {
                        dptr->keyMap.insertMulti(key, loader);
                        ++keyUsageCount;
                    }
                }
            }
            if (keyUsageCount || keys.isEmpty())
                dptr->pluginLoaderList += loader;
            else
                loader->deleteLater();
        }
    }
#else
    if (dfm_debug_component()) {
        qDebug() << "PreviewPluginLoader::PreviewPluginLoader() ignoring" << dptr->iid
                 << "since plugins are disabled in static builds";
    }
#endif
}

void PreviewPluginLoader::refreshAll()
{
    QMutexLocker locker(qt_factoryloader_mutex());
    QList<PreviewPluginLoader *> *loaders = qt_factory_loaders();
    for (QList<PreviewPluginLoader *>::const_iterator it = loaders->constBegin();
         it != loaders->constEnd(); ++it) {
        (*it)->update();
    }
}
