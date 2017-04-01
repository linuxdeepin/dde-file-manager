#include "diskcontrolwidget.h"
#include "diskcontrolitem.h"
#include "gvfsmountmanager.h"
#include "dfmsetting.h"
#include <QDebug>
#include <QProcess>
#include <QThreadPool>
#include <QtConcurrent>

#define WIDTH           300

DiskControlWidget::DiskControlWidget(QWidget *parent)
    : QScrollArea(parent),

      m_centralLayout(new QVBoxLayout),
      m_centralWidget(new QWidget)
{
    m_centralWidget->setLayout(m_centralLayout);
    m_centralWidget->setFixedWidth(WIDTH);

    setWidget(m_centralWidget);
    setFixedWidth(WIDTH);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color:transparent;");
    m_dfmsettings = DFMSetting::instance();
    m_gvfsMountManager = GvfsMountManager::instance();
    m_gvfsMountManager->setAutoMountSwitch(true);
    initConnect();
}

void DiskControlWidget::initConnect()
{
    connect(m_gvfsMountManager, &GvfsMountManager::loadDiskInfoFinished, this, &DiskControlWidget::onDiskListChanged);
    connect(m_gvfsMountManager, &GvfsMountManager::mount_added, this,  &DiskControlWidget::onMount_added);
    connect(m_gvfsMountManager, &GvfsMountManager::mount_removed, this, &DiskControlWidget::onMount_removed);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_added, this, &DiskControlWidget::onVolume_added);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_removed, this, &DiskControlWidget::onVolume_removed);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_changed, this, &DiskControlWidget::onVolume_changed);
}

void DiskControlWidget::startMonitor()
{

    QtConcurrent::run(QThreadPool::globalInstance(), m_gvfsMountManager,
                                             &GvfsMountManager::startMonitor);
}

void DiskControlWidget::unmountAll()
{
    foreach (const QDiskInfo& info, GvfsMountManager::DiskInfos) {
        unmountDisk(info.id());
    }
}

void DiskControlWidget::onDiskListChanged()
{
    qDebug() << "===============" << GvfsMountManager::DiskInfos;
    while (QLayoutItem *item = m_centralLayout->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    int mountedCount = 0;
    for (auto info : GvfsMountManager::DiskInfos)
    {
        if (info.mounted_root_uri().isEmpty())
            continue;
        else
            ++mountedCount;

        DiskControlItem *item = new DiskControlItem(info, this);

        connect(item, &DiskControlItem::requestUnmount, this, &DiskControlWidget::unmountDisk);

        m_centralLayout->addWidget(item);
    }
    emit diskCountChanged(mountedCount);

    const int contentHeight = mountedCount * 70;
    const int maxHeight = std::min(contentHeight, 70 * 6);

    m_centralWidget->setFixedHeight(contentHeight);
    setFixedHeight(maxHeight);
}

void DiskControlWidget::onMount_added(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::onMount_removed(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::onVolume_added(const QDiskInfo &diskInfo)
{
    onDiskListChanged();

    GvfsMountManager* gvfsMountManager = GvfsMountManager::instance();
    DFMSetting* globalSetting = DFMSetting::instance();
    qDebug() << "AutoMountSwitch:" << m_gvfsMountManager->getAutoMountSwitch();
    qDebug() << "isAutoMount:" << globalSetting->isAutoMount();
    qDebug() << "isAutoMountAndOpen:" << globalSetting->isAutoMountAndOpen();
    if (m_gvfsMountManager->getAutoMountSwitch()){
        if(globalSetting->isAutoMount() && !globalSetting->isAutoMountAndOpen()){
            gvfsMountManager->mount(diskInfo);
        }else if (globalSetting->isAutoMountAndOpen()){
            gvfsMountManager->mount(diskInfo);
            QProcess::startDetached("gvfs-open", {"computer:///"});
        }
    }
}

void DiskControlWidget::onVolume_removed(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::onVolume_changed(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::unmountDisk(const QString &diskId) const
{
    m_gvfsMountManager->unmount(diskId);
}
