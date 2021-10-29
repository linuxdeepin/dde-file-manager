/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "diskmountplugin.h"
#include "tipswidget.h"
#include "diskpluginitem.h"
#include "diskcontrolwidget.h"
#include "pluginsidecar.h"

#include <DApplication>
#include <QGSettings>

static const char * const OPEN = "open";
static const char * const EJECT_ALL = "eject_all";

DWIDGET_USE_NAMESPACE

/*!
 * \class DiskMountPlugin
 *
 * \brief DiskMountPlugin is a plugin for dde-dock
 * detail info : https://github.com/linuxdeepin/dde-dock/blob/master/plugins/plugin-guide/plugins-developer-guide.md
 */

DiskMountPlugin::DiskMountPlugin(QObject *parent)
    : QObject(parent),
      tipsLabel(new TipsWidget),
      diskPluginItem(new DiskPluginItem)
{
    diskPluginItem->setVisible(false);
    tipsLabel->setObjectName("diskmount");
    tipsLabel->setVisible(false);
    tipsLabel->setText(tr("Disk"));
}

const QString DiskMountPlugin::pluginName() const
{
    return "disk-mount";
}

void DiskMountPlugin::init(PluginProxyInterface *proxyInter)
{
    QString &&applicationName = qApp->applicationName();
    qApp->setApplicationName("dde-disk-mount-plugin");
    qApp->loadTranslator();
    qApp->setApplicationName(applicationName);

    std::call_once(DiskMountPlugin::onceFlag(), [this, proxyInter] () {
        setProxyInter(proxyInter); // `m_proxyInter` from Base class `PluginsItemInterface`
        SidecarInstance.connectToServer();
        initCompoments();
        diskPluginItem->setDockDisplayMode(displayMode());
    });
}

QWidget *DiskMountPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return diskPluginItem;
}

QWidget *DiskMountPlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return tipsLabel;
}

QWidget *DiskMountPlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return diskControlApplet;
}

const QString DiskMountPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;
    items.reserve(2);

    QGSettings settings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");

    if (settings.get("filemanager-integration").toBool()) {
        QMap<QString, QVariant> open;
        open["itemId"] = OPEN;
        open["itemText"] = tr("Open");
        open["isActive"] = true;
        items.push_back(open);
    }

    QMap<QString, QVariant> ejectAll;
    ejectAll["itemId"] = EJECT_ALL;
    ejectAll["itemText"] = tr("Eject all");
    ejectAll["isActive"] = true;
    items.push_back(ejectAll);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void DiskMountPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey)
    Q_UNUSED(checked)

    if (menuId == OPEN)
        QProcess::startDetached("gio", QStringList() << "open" << "computer:///");
    else if (menuId == EJECT_ALL)
        SidecarInstance.invokeDetachAllMountedDevices();
}

int DiskMountPlugin::itemSortKey(const QString &itemKey)
{
    QString &&key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    int ret = proxyInter()->getValue(this, key, 0).toInt(); // dde-dock默认设置为0
    qDebug() << "itemSortKey [key:" << key << "," << ret << "] for :" << itemKey;
    return ret;
}

void DiskMountPlugin::setSortKey(const QString &itemKey, const int order)
{
    QString &&key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    proxyInter()->saveValue(this, key, order);
    qDebug() << "setSortKey [key:" << key << "," << order << "] for :" << itemKey;
}

void DiskMountPlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == DISK_MOUNT_KEY) {
        diskPluginItem->updateIcon();
    }
}

void DiskMountPlugin::diskCountChanged(const int count)
{
    if (pluginAdded == bool(count))
        return;

    pluginAdded = bool(count);

    if (pluginAdded)
        proxyInter()->itemAdded(this, DISK_MOUNT_KEY);
    else
        proxyInter()->itemRemoved(this, DISK_MOUNT_KEY);
}

void DiskMountPlugin::initCompoments()
{
    diskControlApplet = new DiskControlWidget;
    diskControlApplet->setObjectName("disk-mount");
    diskControlApplet->setVisible(false);

    connect(diskControlApplet, &DiskControlWidget::diskCountChanged, this, &DiskMountPlugin::diskCountChanged);
    // auto mount and monitor work in service process
    diskControlApplet->initListByMonitorState();
}

void DiskMountPlugin::displayModeChanged(const Dock::DisplayMode mode)
{
    diskPluginItem->setDockDisplayMode(mode);
}

PluginProxyInterface *DiskMountPlugin::proxyInter() const
{
    // `m_proxyInter` from Base class `PluginsItemInterface`
    return m_proxyInter;
}

void DiskMountPlugin::setProxyInter(PluginProxyInterface *proxy)
{
    // `m_proxyInter` from Base class `PluginsItemInterface`
    m_proxyInter = proxy;
}

std::once_flag &DiskMountPlugin::onceFlag()
{
    static std::once_flag flag;
    return flag;
}
