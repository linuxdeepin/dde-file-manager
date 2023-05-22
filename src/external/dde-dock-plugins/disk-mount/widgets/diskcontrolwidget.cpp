// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskcontrolwidget.h"
#include "diskcontrolitem.h"
#include "device/dattachedblockdevice.h"
#include "device/dattachedprotocoldevice.h"
#include "device/devicewatcherlite.h"

#include <DGuiApplicationHelper>
#include <DDBusSender>
#include <DDesktopServices>
#include <QScrollBar>
#include <QLabel>
#include <QSharedPointer>
#include <QTimer>

static const int kWidth = 300;

/*!
 * \class DiskControlWidget
 *
 * \brief DiskControlWidget is DiskMountPlugin::itemPopupApplet
 * The control pops up after the left mouse button click on the main control of the plugin
 */

DiskControlWidget::DiskControlWidget(QWidget *parent)
    : QScrollArea(parent),
      centralLayout(new QVBoxLayout),
      centralWidget(new QWidget)
{
    this->setObjectName("DiskControlWidget-QScrollArea");
    initializeUi();
    initConnection();
}

void DiskControlWidget::initListByMonitorState()
{
    onDiskListChanged();
}

void DiskControlWidget::initializeUi()
{
    std::call_once(DiskControlWidget::initOnceFlag(), [this]() {
        centralWidget->setLayout(centralLayout);
        centralWidget->setFixedWidth(kWidth);
        centralLayout->setMargin(0);
        centralLayout->setSpacing(0);
        setWidget(centralWidget);
        setFixedWidth(kWidth);
        setFrameShape(QFrame::NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        verticalScrollBar()->setSingleStep(7);
        centralWidget->setAutoFillBackground(false);
        viewport()->setAutoFillBackground(false);
        paintUi();
    });
}

void DiskControlWidget::initConnection()
{
    // When the system theme changes,
    // refreshes the list of controls to fit the text color under the new theme
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged, this, &DiskControlWidget::onDiskListChanged);

    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::blockDriveAdded, this, [this]() {
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceAdded);
        onDiskListChanged();
    });
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::blockDriveRemoved, this, [this]() {
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceRemoved);
        notifyMessage(QObject::tr("The device has been safely removed"));
        onDiskListChanged();
    });
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::blockDeviceMounted, this, &DiskControlWidget::onDiskListChanged);
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::blockDeviceUnmounted, this, &DiskControlWidget::onDiskListChanged);
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::blockFileSystemAdded, this, &DiskControlWidget::onDiskListChanged);
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::blockFileSystemRemoved, this, &DiskControlWidget::onDiskListChanged);
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::hintIgnoreChanged, this, &DiskControlWidget::onDiskListChanged);

    // the moment the protocolDeviceMounted signal emitted, query info of device may get an empty object
    // so delay to query info of protocol device.
    auto delayChange = [this] { QTimer::singleShot(2000, this, [this] { onDiskListChanged(); }); };
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::protocolDeviceMounted, this, delayChange);
    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::protocolDeviceUnmounted, this, &DiskControlWidget::onDiskListChanged);

    connect(DeviceWatcherLite::instance(), &DeviceWatcherLite::operationFailed, this, &DiskControlWidget::onDeviceBusy);
}

void DiskControlWidget::removeWidgets()
{
    while (QLayoutItem *item = centralLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
}

void DiskControlWidget::paintUi()
{
    QVBoxLayout *headerLay = new QVBoxLayout(this);
    QWidget *header = new QWidget(this);
    header->setLayout(headerLay);
    headerLay->setSpacing(0);
    headerLay->setContentsMargins(20, 9, 0, 8);
    QLabel *headerTitle = new QLabel(tr("Disks"), this);
    QFont f = headerTitle->font();
    f.setPixelSize(20);
    f.setWeight(QFont::Medium);
    headerTitle->setFont(f);
    QPalette pal = headerTitle->palette();
    QColor color = Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::LightType
            ? Qt::black
            : Qt::white;
    pal.setColor(QPalette::WindowText, color);
    headerTitle->setPalette(pal);

    headerLay->addWidget(headerTitle);
    centralLayout->addWidget(header);

    addSeparateLineUi(2);
    int mountedCount = addBlockDevicesItems() + addProtocolDevicesItems();

    // remove last seperate line
    QLayoutItem *last = centralLayout->takeAt(centralLayout->count() - 1);
    if (last) {
        delete last->widget();
        delete last;
    }

    emit diskCountChanged(mountedCount);

    const int contentHeight = mountedCount * 70 + 46;
    const int maxHeight = std::min(contentHeight, 70 * 6);
    centralWidget->setFixedHeight(contentHeight);
    setFixedHeight(maxHeight);

    verticalScrollBar()->setPageStep(maxHeight);
    verticalScrollBar()->setMaximum(contentHeight - maxHeight);
}

void DiskControlWidget::addSeparateLineUi(int width)
{
    QFrame *line = new QFrame(this);
    line->setLineWidth(width);
    line->setFrameStyle(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    centralLayout->addWidget(line);
}

int DiskControlWidget::addBlockDevicesItems()
{
    int mountedCount = 0;

    QStringList &&list = DeviceWatcherLite::instance()->allMountedRemovableBlocks();
    mountedCount = addItems(list, true);

    return mountedCount;
}

int DiskControlWidget::addProtocolDevicesItems()
{
    int mountedCount = 0;

    QStringList &&list = DeviceWatcherLite::instance()->allMountedProtocols();
    mountedCount = addItems(list, false);

    return mountedCount;
}

int DiskControlWidget::addItems(const QStringList &list, bool isBlockDevice)
{
    int mountedCount = 0;

    for (auto &&id : list) {
        QSharedPointer<DAttachedDevice> dev;
        if (isBlockDevice)
            dev.reset(new DAttachedBlockDevice(id));
        else {
            // do not show local vfs mounts. smb is mounted at /media/$USER/smbmounts so it should be displayed.
            if (id.startsWith("file://") && !id.contains(QRegularExpression("^file:///media/[\\s\\S]*/smbmounts"))) {   // TODO(xust), smb's mount point might be changed later.
                qDebug() << "protocol device is ignored: " << id;
                continue;
            }
            dev.reset(new DAttachedProtocolDevice(id));
        }
        dev->query();
        if (dev->isValid()) {
            mountedCount++;
            DiskControlItem *item = new DiskControlItem(dev, this);
            centralLayout->addWidget(item);
            addSeparateLineUi(1);
        }
    }

    return mountedCount;
}

DDialog *DiskControlWidget::showQueryScanningDialog(const QString &title)
{
    DDialog *d = new DDialog;
    d->setTitle(title);
    d->setAttribute(Qt::WA_DeleteOnClose);
    Qt::WindowFlags flags = d->windowFlags();
    d->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d->setIcon(QIcon::fromTheme("dialog-warning"));
    d->addButton(QObject::tr("Cancel", "button"));
    d->addButton(QObject::tr("Stop", "button"), true, DDialog::ButtonWarning);   // 终止
    d->setMaximumWidth(640);
    d->show();
    return d;
}

void DiskControlWidget::handleWhetherScanning(const QString &method, const QString &id)
{
    if (!id.isEmpty() && (method == "unmount" || method == "detach")) {
        //        DevProxyMng->detachBlockDevice(id);
    } else if (method == "detach_all") {
        //        DevProxyMng->detachAllDevices();
    } else {
        qWarning() << "[disk-mount] unknow method: " << method << "or id: " << id;
    }
}

void DiskControlWidget::notifyMessage(const QString &msg)
{
    DDBusSender()
            .service("org.freedesktop.Notifications")
            .path("/org/freedesktop/Notifications")
            .interface("org.freedesktop.Notifications")
            .method(QString("Notify"))
            .arg(QString("dde-file-manager"))
            .arg(static_cast<uint>(0))
            .arg(QString("media-eject"))
            .arg(msg)
            .arg(QString())
            .arg(QStringList())
            .arg(QVariantMap())
            .arg(5000)
            .call();
}

void DiskControlWidget::notifyMessage(const QString &title, const QString &msg)
{
    DDBusSender()
            .service("org.freedesktop.Notifications")
            .path("/org/freedesktop/Notifications")
            .interface("org.freedesktop.Notifications")
            .method(QString("Notify"))
            .arg(QString("dde-file-manager"))
            .arg(static_cast<uint>(0))
            .arg(QString("media-eject"))
            .arg(title)
            .arg(msg)
            .arg(QStringList())
            .arg(QVariantMap())
            .arg(5000)
            .call();
}

std::once_flag &DiskControlWidget::initOnceFlag()
{
    static std::once_flag flag;
    return flag;
}

std::once_flag &DiskControlWidget::retryOnceFlag()
{
    static std::once_flag flag;
    return flag;
}

void DiskControlWidget::onDiskListChanged()
{
    removeWidgets();
    paintUi();
}

/*!
 * \brief show as dialog, then call interfaces
 * \param method: "unmount", "detach", "detach_all"
 * \param id
 */
void DiskControlWidget::onAskStopScanning(const QString &method, const QString &id)
{
    DDialog *d = showQueryScanningDialog(QObject::tr("Scanning the device, stop it?"));

    connect(d, &DDialog::buttonClicked, this, [this, id, method](int index, const QString &text) {
        Q_UNUSED(text);
        if (index == 1)   // user clicked stop
            handleWhetherScanning(method, id);
        else
            qInfo() << "[disk-mount] Continue scanning, status: " << method;
    });
}

void DiskControlWidget::onDeviceBusy(int action)
{
    switch (action) {
    case DeviceWatcherLite::kPowerOff:
        notifyMessage(tr("The device was not safely removed"),
                      tr("Click \"Safely Remove\" and then disconnect it next time"));
        break;
    case DeviceWatcherLite::kUnmount:
        notifyMessage(tr("Disk is busy, cannot unmount now"));
        break;
    case DeviceWatcherLite::kEject:
        notifyMessage(tr("The device is busy, cannot eject now"));
        break;
    default:
        qWarning() << "[disk-mount]: Unknown action: " << action;
    }
}
