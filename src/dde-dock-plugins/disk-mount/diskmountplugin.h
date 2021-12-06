/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DISKMOUNTPLUGIN_H
#define DISKMOUNTPLUGIN_H

#include "pluginsiteminterface.h"

#include <QObject>

class TipsWidget;
class DiskPluginItem;
class DiskControlWidget;

class DiskMountPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "disk-mount.json")

public:
    explicit DiskMountPlugin(QObject *parent = nullptr);

    const QString pluginName() const override;
    void init(PluginProxyInterface *proxyInter) override;
    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;
    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;
    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;
    void refreshIcon(const QString &itemKey) override;

public slots:
    void diskCountChanged(const int count);

private:
    void initCompoments();
    void displayModeChanged(const Dock::DisplayMode mode) override;
    PluginProxyInterface *proxyInter() const;
    void setProxyInter(PluginProxyInterface *proxy);
    static std::once_flag &onceFlag();

private:
    bool pluginAdded { false };

    TipsWidget *tipsLabel { nullptr };
    DiskPluginItem *diskPluginItem { nullptr };
    DiskControlWidget *diskControlApplet { nullptr };
};

#endif   // DISKMOUNTPLUGIN_H
