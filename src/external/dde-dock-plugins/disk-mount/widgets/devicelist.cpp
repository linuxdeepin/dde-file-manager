// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicelist.h"
#include "deviceitem.h"
#include "device/dockitemdatamanager.h"

#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <algorithm>

Q_DECLARE_LOGGING_CATEGORY(logAppDock)

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

DeviceList::DeviceList(QWidget *parent)
    : QScrollArea(parent)
{
    this->setObjectName("DiskControlWidget-QScrollArea");
    initUI();
    initConnect();
}

void DeviceList::showEvent(QShowEvent *e)
{
    updateHeight();

    // 立即刷新设备容量（显示最新数据）
    qCDebug(logAppDock) << "DeviceList shown, requesting immediate usage refresh";
    manager->refreshUsage();

    // 订阅容量监控（引用计数 +1，持续更新）
    qCDebug(logAppDock) << "DeviceList shown, subscribing to usage monitoring";
    manager->subscribeUsageMonitoring();

    QScrollArea::showEvent(e);
}

void DeviceList::hideEvent(QHideEvent *e)
{
    // 取消订阅容量监控（引用计数 -1）
    qCDebug(logAppDock) << "DeviceList hidden, unsubscribing from usage monitoring";
    manager->unsubscribeUsageMonitoring();

    QScrollArea::hideEvent(e);
}

void DeviceList::addDevice(const DockItemData &item)
{
    if (deviceItems.contains(item.id)) {
        qCDebug(logAppDock) << "Device already exists, removing before re-adding:" << item.id;
        removeDevice(item.id);
    }

    auto devItem = new DeviceItem(item, this);
    connect(devItem, &DeviceItem::requestEject,
            this, &DeviceList::ejectDevice);
    deviceItems.insert(item.id, devItem);
    sortKeys.insert(item.id, item.sortKey);

    QStringList order = sortKeys.values();
    std::sort(order.begin(), order.end());

    deviceLay->insertWidget(order.indexOf(item.sortKey), devItem);

    qCInfo(logAppDock) << "Added device item:" << item.id << devItem;
    updateHeight();
}

void DeviceList::removeDevice(const QString &id)
{
    auto wgt = deviceItems.value(id, nullptr);
    if (!wgt) {
        qCDebug(logAppDock) << "Device not found for removal:" << id;
        return;
    }

    qCInfo(logAppDock) << "Removing device item:" << id << wgt;
    deviceLay->removeWidget(wgt);
    delete wgt;
    wgt = nullptr;
    deviceItems.remove(id);
    sortKeys.remove(id);
    updateHeight();
}

void DeviceList::ejectDevice(const QString &id)
{
    qCInfo(logAppDock) << "Ejecting device:" << id;
    DockItemDataManager::instance()->ejectDevice(id);
}

void DeviceList::initUI()
{
    deviceLay = new QVBoxLayout();
    deviceLay->setContentsMargins(0, 0, 0, 0);
    deviceLay->setSpacing(0);

    QVBoxLayout *mainLay = new QVBoxLayout();
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    mainLay->setSizeConstraint(QLayout::SetFixedSize);

    mainLay->addWidget(createHeader());
    mainLay->addLayout(deviceLay);

    QWidget *content = new QWidget(this);
    content->setLayout(mainLay);
    setWidget(content);

    setFixedWidth(kDockPluginWidth);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setSingleStep(7);
    content->setAutoFillBackground(false);
    setWidgetResizable(true);
    setMaximumHeight(420);
}

void DeviceList::initConnect()
{
    manager = DockItemDataManager::instance();
    connect(manager, &DockItemDataManager::mountAdded,
            this, &DeviceList::addDevice);
    connect(manager, &DockItemDataManager::mountRemoved,
            this, &DeviceList::removeDevice);
    connect(manager, &DockItemDataManager::usageUpdated,
            this, [this](auto id, auto used) {
                auto item = deviceItems.value(id, nullptr);
                auto devItem = dynamic_cast<DeviceItem *>(item);
                if (devItem) devItem->updateUsage(used);
            });
    manager->initialize();
}

void DeviceList::updateHeight()
{
    const int itemHeight = deviceItems.isEmpty() ? 0 : deviceItems.first()->sizeHint().height();
    const int headerHeight = headerWidget->sizeHint().height();
    int contentHeight = headerHeight + itemHeight * deviceItems.count();
    if (contentHeight > 420)
        contentHeight = 420;
    resize(width(), contentHeight);
}

QWidget *DeviceList::createHeader()
{
    headerWidget = new QWidget(this);
    headerWidget->setFixedWidth(kDockPluginWidth);
    QVBoxLayout *lay = new QVBoxLayout();
    lay->setSpacing(0);
    lay->setContentsMargins(20, 9, 0, 8);

    QVBoxLayout *headerLay = new QVBoxLayout();
    headerLay->setContentsMargins(0, 0, 0, 0);
    headerLay->setSpacing(0);
    headerWidget->setLayout(headerLay);

    QLabel *title = new QLabel(tr("Disks"), this);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T5, QFont::DemiBold);
    lay->addWidget(title);

    auto line = DeviceItem::createSeparateLine(1);
    line->setParent(this);
    headerLay->addLayout(lay);
    headerLay->addWidget(line);

    auto setTextColor = [title](DGuiApplicationHelper::ColorType) {
        QPalette pal = title->palette();
        QColor color = Qt::white;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            color = Qt::black;
        // pal.setColor(QPalette::WindowText, color);
        title->setPalette(pal);
    };

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            title, [=](auto type) { setTextColor(type); });
    setTextColor(DGuiApplicationHelper::instance()->themeType());
    return headerWidget;
}
