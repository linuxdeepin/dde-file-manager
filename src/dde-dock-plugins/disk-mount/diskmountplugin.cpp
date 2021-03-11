/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include <DApplication>
#include <dgiosettings.h>

#define OPEN        "open"
#define UNMOUNT_ALL "unmount_all"

DWIDGET_USE_NAMESPACE

DiskMountPlugin::DiskMountPlugin(QObject *parent)
    : QObject(parent),

      m_pluginAdded(false),
      m_pluginLoaded(false),
      m_usingAppLoader(true),

      m_tipsLabel(new TipsWidget),
      m_diskPluginItem(new DiskPluginItem),
      m_diskControlApplet(nullptr)
{
    qDebug() << "===============init=============";
    m_diskPluginItem->setVisible(false);

    m_tipsLabel->setObjectName("diskmount");
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setText(tr("Disk"));
}

DiskMountPlugin::DiskMountPlugin(bool usingAppLoader, QObject *parent)
    : QObject(parent),

      m_pluginAdded(false),
      m_pluginLoaded(false),
      m_usingAppLoader(usingAppLoader),

      m_tipsLabel(new TipsWidget),
      m_diskPluginItem(new DiskPluginItem),
      m_diskControlApplet(nullptr)
{
    qDebug() << "===============init=============";
    m_diskPluginItem->setVisible(false);

    m_tipsLabel->setObjectName("diskmount");
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setText(tr("Disk"));
}

const QString DiskMountPlugin::pluginName() const
{
    return "disk-mount";
}

void DiskMountPlugin::init(PluginProxyInterface *proxyInter)
{
    // blumia: we are using i10n translation from DFM so...     qApp->loadTranslator();
    QString applicationName = qApp->applicationName();
    qApp->setApplicationName("dde-disk-mount-plugin");
    if(m_usingAppLoader){
        qDebug() << qApp->loadTranslator();
    }
    qApp->setApplicationName(applicationName);
    qDebug() << "===============init==proxyInter===========";
    m_proxyInter = proxyInter;

    if (m_pluginLoaded) {
        qDebug() << "disk mount plugin has been loaded! return";
        return;
    }

    m_pluginLoaded = true;

    initCompoments();
    m_diskPluginItem->setDockDisplayMode(displayMode());
}

QWidget *DiskMountPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_diskPluginItem;
}

QWidget *DiskMountPlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_tipsLabel;
}

QWidget *DiskMountPlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_diskControlApplet;
}

const QString DiskMountPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;
    items.reserve(2);

    DGioSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");

    if (gsettings.value("filemanager-integration").toBool()) {
        QMap<QString, QVariant> open;
        open["itemId"] = OPEN;
        open["itemText"] = tr("Open");
        open["isActive"] = true;
        items.push_back(open);
    }

    QMap<QString, QVariant> unmountAll;
    unmountAll["itemId"] = UNMOUNT_ALL;
    unmountAll["itemText"] = tr("Eject all");
    unmountAll["isActive"] = true;
    items.push_back(unmountAll);

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
    else if (menuId == UNMOUNT_ALL)
        m_diskControlApplet->unmountAll();
}

int DiskMountPlugin::itemSortKey(const QString &itemKey)
{
    const QString &key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    int ret = m_proxyInter->getValue(this, key, 0).toInt(); // dde-dock默认设置为0
    qDebug() << "itemSortKey [key:" << key << "," << ret << "] for :" << itemKey;
    return ret;
}

void DiskMountPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString &key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
    qDebug() << "setSortKey [key:" << key << "," << order << "] for :" << itemKey;
}

void DiskMountPlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == DISK_MOUNT_KEY) {
        m_diskPluginItem->updateIcon();
    }
}

void DiskMountPlugin::initCompoments()
{
    m_diskControlApplet = new DiskControlWidget;
    m_diskControlApplet->setObjectName("disk-mount");
    m_diskControlApplet->setVisible(false);

    connect(m_diskControlApplet, &DiskControlWidget::diskCountChanged, this, &DiskMountPlugin::diskCountChanged);
    m_diskControlApplet->doStartupAutoMount();
    m_diskControlApplet->startMonitor();
}

void DiskMountPlugin::displayModeChanged(const Dock::DisplayMode mode)
{
    m_diskPluginItem->setDockDisplayMode(mode);
}

void DiskMountPlugin::diskCountChanged(const int count)
{
    qDebug() << count << m_pluginAdded;
    if (m_pluginAdded == bool(count))
        return;

    m_pluginAdded = bool(count);

    if (m_pluginAdded)
        m_proxyInter->itemAdded(this, DISK_MOUNT_KEY);
    else
        m_proxyInter->itemRemoved(this, DISK_MOUNT_KEY);
}
