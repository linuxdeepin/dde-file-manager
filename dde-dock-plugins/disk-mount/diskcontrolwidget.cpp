#include "diskcontrolwidget.h"
#include "diskcontrolitem.h"
#include "dde-file-manager/gvfs/gvfsmountmanager.h"
#include <QDebug>

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
    m_gvfsMountManager = GvfsMountManager::instance();
    initConnect();
}

void DiskControlWidget::initConnect()
{
    connect(m_gvfsMountManager, &GvfsMountManager::loadDiskInfoFinished, this, &DiskControlWidget::onDiskListChanged);
    connect(m_gvfsMountManager, &GvfsMountManager::mount_added, this,  &DiskControlWidget::onDiskListChanged);
    connect(m_gvfsMountManager, &GvfsMountManager::mount_removed, this, &DiskControlWidget::onDiskListChanged);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_added, this, &DiskControlWidget::onDiskListChanged);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_removed, this, &DiskControlWidget::onDiskListChanged);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_changed, this, &DiskControlWidget::onDiskListChanged);
}

void DiskControlWidget::startMonitor()
{
    m_gvfsMountManager->startMonitor();
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

void DiskControlWidget::unmountDisk(const QString &diskId) const
{
    m_gvfsMountManager->unmount(diskId);
}
