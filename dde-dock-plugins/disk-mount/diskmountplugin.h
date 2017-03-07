#ifndef DISKMOUNTPLUGIN_H
#define DISKMOUNTPLUGIN_H

#include <QLabel>

#include "pluginsiteminterface.h"
#include "diskcontrolwidget.h"
#include "diskpluginitem.h"

class DiskMountPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "disk-mount.json")

public:
    explicit DiskMountPlugin(QObject *parent = 0);

    const QString pluginName() const;
    void init(PluginProxyInterface *proxyInter);

    QWidget *itemWidget(const QString &itemKey);
    QWidget *itemTipsWidget(const QString &itemKey);
    QWidget *itemPopupApplet(const QString &itemKey);

    const QString itemContextMenu(const QString &itemKey);
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked);

private:
    void initCompoments();

    void displayModeChanged(const Dock::DisplayMode mode);

private slots:
    void diskCountChanged(const int count);

private:
    bool m_pluginAdded;

    QLabel *m_tipsLabel;
    DiskPluginItem *m_diskPluginItem;
    DiskControlWidget *m_diskControlApplet;
};

#endif // DISKMOUNTPLUGIN_H
