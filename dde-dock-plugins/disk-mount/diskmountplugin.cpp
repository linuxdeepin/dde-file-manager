#include "diskmountplugin.h"
#include "dfmglobal.h"

#define OPEN        "open"
#define UNMOUNT_ALL "unmount_all"

DiskMountPlugin::DiskMountPlugin(QObject *parent)
    : QObject(parent),

      m_pluginAdded(false),

      m_tipsLabel(new QLabel),
      m_diskPluginItem(new DiskPluginItem),
      m_diskControlApplet(nullptr)
{
    qDebug() << "===============init=============";
    m_diskPluginItem->setVisible(false);

    m_tipsLabel->setObjectName("diskmount");
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setText(tr("Disk"));
    m_tipsLabel->setStyleSheet("color:white;"
                               "padding:5px 10px;");

    connect(m_diskPluginItem, &DiskPluginItem::requestContextMenu, [this] {m_proxyInter->requestContextMenu(this, QString());});
}

const QString DiskMountPlugin::pluginName() const
{
    return "disk-mount";
}

void DiskMountPlugin::init(PluginProxyInterface *proxyInter)
{
    qDebug() << "===============init== proxyInter===========";
    m_proxyInter = proxyInter;

    initCompoments();
    m_diskPluginItem->setDockDisplayMode(displayMode());

    DFMGlobal::installTranslator();
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

    QMap<QString, QVariant> open;
    open["itemId"] = OPEN;
    open["itemText"] = tr("Open");
    open["isActive"] = true;
    items.push_back(open);

    QMap<QString, QVariant> unmountAll;
    unmountAll["itemId"] = UNMOUNT_ALL;
    unmountAll["itemText"] = tr("Unmount all");
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
        QProcess::startDetached("gvfs-open", QStringList() << "computer:///");
    else if (menuId == UNMOUNT_ALL)
        m_diskControlApplet->unmountAll();
}

void DiskMountPlugin::initCompoments()
{
    m_diskControlApplet = new DiskControlWidget;
    m_diskControlApplet->setObjectName("dist-mount");
    m_diskControlApplet->setVisible(false);

    connect(m_diskControlApplet, &DiskControlWidget::diskCountChanged, this, &DiskMountPlugin::diskCountChanged);
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
        m_proxyInter->itemAdded(this, QString());
    else
        m_proxyInter->itemRemoved(this, QString());
}
