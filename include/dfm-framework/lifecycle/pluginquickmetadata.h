// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINQUICKMETADATA_H
#define PLUGINQUICKMETADATA_H

#include <dfm-framework/dfm_framework_global.h>

#include <QUrl>
#include <QString>

DPF_BEGIN_NAMESPACE

class PluginQuickData;
class PluginQuickMetaData final
{
    friend class PluginQuickData;
    friend class PluginQuickMetaDataCreator;
    friend class PluginManager;
    friend class PluginManagerPrivate;
    friend class PluginMetaObject;
    friend Q_CORE_EXPORT QDebug operator<<(QDebug, const PluginQuickMetaData &);

public:
    PluginQuickMetaData();
    PluginQuickMetaData(const QUrl &url,
                        const QString &id,
                        const QString &plugin,
                        const QString &type,
                        const QString &parent,
                        const QString &applet);

    QUrl url() const;
    QString id() const;
    QString plugin() const;
    QString type() const;
    QString parent() const;
    QString applet() const;

private:
    QScopedPointer<PluginQuickData> d;
    Q_DISABLE_COPY(PluginQuickMetaData)
};

using PluginQuickMetaPtr = QSharedPointer<PluginQuickMetaData>;

class PluginQuickMetaDataCreator final
{
public:
    PluginQuickMetaDataCreator();

    void create(const QString &plugin, const QString &id, const QUrl &url);
    void setType(const QString &type);
    void setParent(const QString &parent);
    void setApplet(const QString &applet);
    PluginQuickMetaPtr take();

private:
    PluginQuickMetaPtr metaPtr;
};

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const DPF_NAMESPACE::PluginQuickMetaData &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const DPF_NAMESPACE::PluginQuickMetaPtr &);
#endif   // QT_NO_DEBUG_STREAM
QT_END_NAMESPACE

DPF_END_NAMESPACE

#endif   // PLUGINQUICKMETADATA_H
