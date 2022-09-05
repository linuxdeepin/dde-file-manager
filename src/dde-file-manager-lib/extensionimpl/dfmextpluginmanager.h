// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTPLUGINMANAGER_H
#define DFMEXTPLUGINMANAGER_H

#include "dfmextpluginloader.h"

#include <QString>
#include <QObject>

class DFMExtMenuImplProxy;
class DFMExtPluginManagerPrivate;
class DFMExtPluginManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DFMExtPluginManager)
    DFMExtPluginManagerPrivate *const d;
public:
    enum State {
        Invalid,
        Scanned,
        Loaded,
        Initialized,
        Shutdown
    };

    enum PluginLogicState {
        Enable,
        Disbale
    };

    using DFMExtMenus = QList<QSharedPointer<DFMEXT::DFMExtMenuPlugin>>;
    using DFMExtEmblemIcons = QList<QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin>>;
    using DFMExtMenuState = QPair<PluginLogicState, QSharedPointer<DFMEXT::DFMExtMenuPlugin>>;
    using DFMExtEmblemState =  QPair<PluginLogicState, QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin>>;
    using DFMExtMenuMap = QMap<QString, DFMExtMenuState>;
    using DFMExtEmblemIconMap = QMap<QString, DFMExtEmblemState>;

    QString pluginDefaultPath();
    void setPluginPaths(const QStringList &paths);
    QStringList pluginPaths() const;
    bool scanPlugins();
    bool loadPlugins();
    bool initPlugins();
    bool monitorPlugins();
    bool shutdownPlugins();
    DFMExtMenus menus() const;
    DFMExtEmblemIcons emblemIcons() const;
    DFMExtMenuImplProxy *pluginMenuProxy();
    State state() const;
    QString errorString() const;
    static DFMExtPluginManager &instance();

signals:
    void extensionPluginCreated(const QString &path);
    void extensionPluginEnable(const QString &path);
    void extensionPluginDisbale(const QString &path);

private:
    explicit DFMExtPluginManager(QObject *parent = nullptr);
};

#endif // DFMEXTPLUGINMANAGER_H
