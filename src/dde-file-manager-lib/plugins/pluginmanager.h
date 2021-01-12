/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QMap>

class MenuInterface;
class PluginManager;
class PropertyDialogExpandInfoInterface;
class ViewInterface;
class PreviewInterface;

class PluginManagerPrivate {

public:
    explicit PluginManagerPrivate(PluginManager* parent);

    QList<PropertyDialogExpandInfoInterface*> expandInfoInterfaces;
    QList<ViewInterface*> viewInterfaces;
    QMap<QString, ViewInterface*> viewInterfacesMap;
    QList<PreviewInterface*> previewInterfaces;

private:
    PluginManager* q_ptr {nullptr};
    Q_DECLARE_PUBLIC(PluginManager)
};


class PluginManager : public QObject
{
    Q_OBJECT

public:
    inline static PluginManager* instance(){
        static PluginManager* instance = new PluginManager();
        return instance;
    }

    static QString PluginDir();


    void loadPlugin();
    QList<PropertyDialogExpandInfoInterface*> getExpandInfoInterfaces();
    QList<ViewInterface*> getViewInterfaces();
    QMap<QString, ViewInterface*> getViewInterfacesMap();
    QList<PreviewInterface*> getPreviewInterfaces();
    ViewInterface* getViewInterfaceByScheme(const QString& scheme);

signals:

public slots:

private:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    QScopedPointer<PluginManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PluginManager)
};

#endif // PLUGINMANAGER_H
