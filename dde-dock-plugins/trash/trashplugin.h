#ifndef TRASHPLUGIN_H
#define TRASHPLUGIN_H

#include "pluginsiteminterface.h"
#include "trashwidget.h"

#include <QLabel>

class TrashPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "trash.json")

public:
    explicit TrashPlugin(QObject *parent = 0);

    const QString pluginName() const;
    void init(PluginProxyInterface *proxyInter);

    QWidget *itemWidget(const QString &itemKey);
    QWidget *itemTipsWidget(const QString &itemKey);
    QWidget *itemPopupApplet(const QString &itemKey);
    const QString itemCommand(const QString &itemKey);
    const QString itemContextMenu(const QString &itemKey);
    void refershIcon(const QString &itemKey);
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked);

    int itemSortKey(const QString &itemKey);
    void displayModeChanged(const Dock::DisplayMode displayMode);

private:
    void showContextMenu();

private:
    TrashWidget *m_trashWidget;
    QLabel *m_tipsLabel;
};

#endif // TRASHPLUGIN_H
