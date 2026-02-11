// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLOADER_P_H
#define PLUGINLOADER_P_H

#include "dfmplugin_filepreview_global.h"

#include <QObject>
#include <QMutex>
#include <QMultiMap>
#include <QPluginLoader>

namespace dfmplugin_filepreview {

inline QString iidKeyLiteral()
{
    return QStringLiteral("IID");
}
#ifdef QT_SHARED
inline QString versionKeyLiteral()
{
    return QStringLiteral("version");
}
#endif
inline QString metaDataKeyLiteral()
{
    return QStringLiteral("MetaData");
}
inline QString keysKeyLiteral()
{
    return QStringLiteral("Keys");
}

class PreviewPluginLoaderPrivate : public QObject
{
    Q_OBJECT
public:
    explicit PreviewPluginLoaderPrivate(QObject *parent = nullptr);
    virtual ~PreviewPluginLoaderPrivate();
    mutable QMutex mutex;
    QByteArray iid;
    QList<QPluginLoader *> pluginLoaderList;
    QMultiMap<QString, QPluginLoader *> keyMap;
    QString suffix;
    Qt::CaseSensitivity cs;
    bool rki = false;
    QStringList loadedPaths;

    static QStringList pluginPaths;
};
}
#endif   //! PLUGINLOADER_P_H
