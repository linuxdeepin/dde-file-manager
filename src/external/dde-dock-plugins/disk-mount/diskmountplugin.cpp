// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskmountplugin.h"
#include "widgets/tipswidget.h"
#include "widgets/diskpluginitem.h"
#include "widgets/diskcontrolwidget.h"
#include "device/devicewatcherlite.h"

#include <DApplication>
#include <QGSettings>

Q_LOGGING_CATEGORY(logAppDock, "log.dock.disk-mount")

static const char *const kOpen = "open";
static const char *const kEjectAll = "eject_all";
static const char *const kDiskMountKey = "mount-item-key";

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
    loadTranslator();
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
    std::call_once(DiskMountPlugin::onceFlag(), [this, proxyInter]() {
        setProxyInter(proxyInter);   // `m_proxyInter` from Base class `PluginsItemInterface`
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

    if (settings.get("filemanagerIntegration").toBool()) {
        QMap<QString, QVariant> open;
        open["itemId"] = kOpen;
        open["itemText"] = tr("Open");
        open["isActive"] = true;
        items.push_back(open);
    }

    QMap<QString, QVariant> ejectAll;
    ejectAll["itemId"] = kEjectAll;
    ejectAll["itemText"] = tr("Eject all");
    ejectAll["isActive"] = true;
    items.push_back(ejectAll);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void DiskMountPlugin::invokedMenuItem(const QString &, const QString &menuId, const bool)
{
    if (menuId == kOpen)
        QProcess::startDetached("gio", QStringList { "open", "computer:///" });
    else if (menuId == kEjectAll)
        DeviceWatcherLite::instance()->detachAllDevices();
}

int DiskMountPlugin::itemSortKey(const QString &itemKey)
{
    QString &&key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    int ret = proxyInter()->getValue(this, key, 0).toInt();   // dde-dock默认设置为0
    qCDebug(logAppDock) << "itemSortKey [key:" << key << "," << ret << "] for :" << itemKey;
    return ret;
}

void DiskMountPlugin::setSortKey(const QString &itemKey, const int order)
{
    QString &&key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    proxyInter()->saveValue(this, key, order);
    qCDebug(logAppDock) << "setSortKey [key:" << key << "," << order << "] for :" << itemKey;
}

void DiskMountPlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == kDiskMountKey)
        diskPluginItem->updateIcon();
}

void DiskMountPlugin::diskCountChanged(const int count)
{
    if (pluginAdded == bool(count))
        return;

    pluginAdded = bool(count);

    if (pluginAdded)
        proxyInter()->itemAdded(this, kDiskMountKey);
    else
        proxyInter()->itemRemoved(this, kDiskMountKey);
}

void DiskMountPlugin::loadTranslator()
{
    QString &&applicationName = qApp->applicationName();
    qApp->setApplicationName("dde-file-manager");
    qApp->loadTranslator();
    qApp->setApplicationName(applicationName);
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

#ifdef COMPILE_ON_V23
QIcon DiskMountPlugin::icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType)
{
    if (dockPart == DockPart::SystemPanel) {
        diskPluginItem->updateIcon();
        return diskPluginItem->getIcon();
    }
    return QIcon();
}

PluginFlags DiskMountPlugin::flags() const
{
    return PluginFlag::Type_Tray | PluginFlag::Attribute_CanDrag | PluginFlag::Attribute_CanInsert;
}
#endif
