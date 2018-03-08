/*
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

#ifndef TRASHPLUGIN_H
#define TRASHPLUGIN_H

#include "pluginsiteminterface.h"
#include "trashwidget.h"

#include <QLabel>
#include <QSettings>

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

    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;
    void displayModeChanged(const Dock::DisplayMode displayMode);

private:
    void showContextMenu();

private:
    TrashWidget *m_trashWidget;
    QLabel *m_tipsLabel;
    QSettings m_settings;
};

#endif // TRASHPLUGIN_H
