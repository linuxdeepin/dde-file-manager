// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

#ifndef COMPILE_ON_V20
    QIcon icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType()) override;
    PluginFlags flags() const override;
#endif

public slots:
    void diskCountChanged(const int count);

private:
    void loadTranslator();
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
