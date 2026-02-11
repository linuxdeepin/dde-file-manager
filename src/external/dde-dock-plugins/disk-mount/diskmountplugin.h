// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKMOUNTPLUGIN_H
#define DISKMOUNTPLUGIN_H

#include <dde-dock/constants.h>

// `DOCK_API_VERSION` added after v2.0.0
#ifdef DOCK_API_VERSION
#    if (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))
#        define USE_DOCK_NEW_INTERFACE
#    endif
#endif

#ifdef USE_DOCK_NEW_INTERFACE
#    include <dde-dock/pluginsiteminterface_v2.h>
#else
#    include <dde-dock/pluginsiteminterface.h>
#endif

#include <QObject>

class TipsWidget;
class DiskPluginItem;
class DeviceList;

#ifdef USE_DOCK_NEW_INTERFACE
class DiskMountPlugin : public QObject, PluginsItemInterfaceV2
{
#else
class DiskMountPlugin : public QObject, PluginsItemInterface
{
#endif
    Q_OBJECT
#ifdef USE_DOCK_NEW_INTERFACE
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "disk-mount2.json")
#else
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "disk-mount.json")
#endif

public:
    explicit DiskMountPlugin(QObject *parent = nullptr);
#ifdef USE_DOCK_NEW_INTERFACE
    Dock::PluginFlags flags() const override
    {
        return Dock::Type_Tray | Dock::Attribute_CanDrag | Dock::Attribute_CanInsert;
    }
#endif

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
    void setDockEntryVisible(bool visible);

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
    DeviceList *diskControlApplet { nullptr };
};

#endif   // DISKMOUNTPLUGIN_H
