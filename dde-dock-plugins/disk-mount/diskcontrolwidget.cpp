/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "diskcontrolwidget.h"
#include "diskcontrolitem.h"
#include "dattachedudisks2device.h"
#include "dattachedvfsdevice.h"
#include "models/dfmrootfileinfo.h"
#include "diskglobal.h"

#include <dgiovolumemanager.h>
#include <dgiomount.h>
#include <dgiofile.h>

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <dfmsettings.h>
#include <dgiosettings.h>
#include <DDesktopServices>
#include <DGuiApplicationHelper>

#include <QDebug>
#include <QProcess>
#include <QThreadPool>
#include <QtConcurrent>
#include <QScrollBar>
#include <QDebug>
#include <DDBusSender>
#define WIDTH           300

DWIDGET_USE_NAMESPACE

DFM_USE_NAMESPACE

DiskControlWidget::DiskControlWidget(QWidget *parent)
    : QScrollArea(parent),
      m_centralLayout(new QVBoxLayout),
      m_centralWidget(new QWidget)
{
    this->setObjectName("DiskControlWidget-QScrollArea");

    m_centralWidget->setLayout(m_centralLayout);
    m_centralWidget->setFixedWidth(WIDTH);

    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);

    m_vfsManager.reset(new DGioVolumeManager);

    setWidget(m_centralWidget);
    setFixedWidth(WIDTH);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setSingleStep(7);
    m_centralWidget->setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);

    m_diskManager = new DDiskManager(this);
    initConnect();
}

DiskControlWidget::~DiskControlWidget()
{

}

void DiskControlWidget::initConnect()
{
    connect(m_diskManager, &DDiskManager::diskDeviceAdded, this, &DiskControlWidget::onDriveConnected);
    connect(m_diskManager, &DDiskManager::blockDeviceAdded, this, &DiskControlWidget::onBlockDeviceAdded);
    connect(m_diskManager, &DDiskManager::diskDeviceRemoved, this, &DiskControlWidget::onDriveDisconnected);
    connect(m_diskManager, &DDiskManager::mountAdded, this, &DiskControlWidget::onMountAdded);
    connect(m_diskManager, &DDiskManager::mountRemoved, this, &DiskControlWidget::onMountRemoved);
    connect(m_diskManager, &DDiskManager::fileSystemAdded, this, &DiskControlWidget::onVolumeAdded);
    connect(m_diskManager, &DDiskManager::fileSystemRemoved, this, &DiskControlWidget::onVolumeRemoved);

    connect(m_vfsManager.data(), &DGioVolumeManager::mountAdded, this, &DiskControlWidget::onVfsMountChanged);
//    connect(m_vfsManager.data(), &DGioVolumeManager::mountChanged, this, &DiskControlWidget::onVfsMountChanged);
    connect(m_vfsManager.data(), &DGioVolumeManager::mountRemoved, this, &DiskControlWidget::onVfsMountChanged);

    // 系统主题改变，重新刷新控件列表以适应新主题下的文字颜色
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged,
            this, [this]{ this->onDiskListChanged(); });
}

DDiskManager*  DiskControlWidget::startMonitor()
{
    m_diskManager->setWatchChanges(true);
    onDiskListChanged();
    return m_diskManager;
}

/*
 *
 * TODO: move this thing into dtkcore or somewhere...
 *       There is also a FileUtils::getKernelParameters() in dde-file-manager.
 * blumia: for writing unit test, try validate the result with `tr ' ' '\n' < /proc/cmdline`
 */
QMap<QString, QString> getKernelParameters()
{
    QFile cmdline("/proc/cmdline");
    cmdline.open(QIODevice::ReadOnly);
    QByteArray content = cmdline.readAll();

    QByteArrayList paraList(content.split(' '));

    QMap<QString, QString> result;
    result.insert("_ori_proc_cmdline", content);

    for (const QByteArray &onePara : paraList) {
        int equalsIdx = onePara.indexOf('=');
        QString key = equalsIdx == -1 ? onePara.trimmed() : onePara.left(equalsIdx).trimmed();
        QString value = equalsIdx == -1 ? QString() : onePara.right(equalsIdx).trimmed();
        result.insert(key, value);
    }

    return result;
}

void DiskControlWidget::doStartupAutoMount()
{
    // check if we are in live system, don't do auto mount if we are in live system.
    static QMap<QString, QString> cmdline = getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live")) {
        m_isInLiveSystem = true;
        return;
    }

    // 插件启动的时候判定是否挂载所有块设备
    m_autoMountEnable = getGsGlobal()->value("GenericAttribute", "AutoMount", false).toBool();
    if (!m_autoMountEnable) {
        return;
    }

    QStringList blDevList = m_diskManager->blockDevices({});
    for (const QString &blDevStr : blDevList) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));

        if (DiskGlobal::isWayLand() && blDevStr.contains(QRegularExpression("/sd[a-c][1-9]*$"))) {
            continue;
        }

        if (blDev->isEncrypted()) continue;
        if (blDev->hintIgnore()) continue;

        QList<QByteArray> mountPoints = blDev->mountPoints();
        if (blDev->hasFileSystem() && blDev->mountPoints().isEmpty()) {
            blDev->mount({{"auth.no_user_interaction", true}});
        }
    }
}

bool isProtectedDevice(DBlockDevice *blk)
{
    DGioSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");
    if (gsettings.value("protect-non-media-mounts").toBool()) {
        QList<QByteArray> mountPoints = blk->mountPoints();
        for (auto &mountPoint : mountPoints) {
            if (!mountPoint.startsWith("/media/")) {
                return true;
            }
        }
    }

    if (gsettings.value("protect-root-device-mounts").toBool()) {
        QStorageInfo qsi("/");
        QStringList rootDevNodes = DDiskManager::resolveDeviceNode(qsi.device(), {});
        if (!rootDevNodes.isEmpty()) {
            if (DDiskManager::createBlockDevice(rootDevNodes.first())->drive() == blk->drive()) {
                return true;
            }
        }
    }

    return false;
}

void DiskControlWidget::unmountAll()
{
    QStringList blockDevices = m_diskManager->blockDevices({});

    QtConcurrent::run([blockDevices]() {
        for (const QString &blDevStr : blockDevices) {
            QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));
            if (isProtectedDevice(blDev.data())) continue;
            if (blDev->hasFileSystem() /* && DFMSetting*/ && !blDev->mountPoints().isEmpty() && !blDev->hintIgnore() && !blDev->hintSystem()) {
                QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blDev->drive()));
                blDev->unmount({});
                qDebug() << "unmountAll" << "removable" <<  diskDev->removable() <<
                         "optical" << diskDev->optical() <<
                         "canPowerOff" << diskDev->canPowerOff() <<
                         "ejectable" << diskDev->ejectable();

                if (diskDev->removable()) {
                    diskDev->eject({});
                    qDebug() << "unmountAll";
                    if (diskDev->lastError().isValid()) {
                        qWarning() << diskDev->lastError().name() << blockDevices;
                        NotifyMsg(tr("The device was not safely removed"), tr("Click \"Safely Remove\" and then disconnect it next time") );
                        continue;
                    }
                }
                if (diskDev->optical()) { // is optical
                    if (diskDev->ejectable()) {
                        diskDev->eject({});
                        if (diskDev->lastError().isValid()) {
                            qWarning() << diskDev->lastError().name() << blockDevices;
                            NotifyMsg(tr("The device was not safely removed"), tr("Click \"Safely Remove\" and then disconnect it next time") );
                        }
                        continue; // fix bug#16936 在 dock 上选择了卸载全部后，还会有U盘未被卸载
                    }
                }

                if (diskDev->canPowerOff()) {
                    diskDev->powerOff({});
                }
            }
        }
    });

    QList<QExplicitlySharedDataPointer<DGioMount> > vfsMounts = getVfsMountList();
    for (auto mount : vfsMounts) {
        if (mount->isShadowed()) {
            continue;
        }
        QExplicitlySharedDataPointer<DGioFile> rootFile = mount->getRootFile();
        QString path = rootFile->path();
        DAttachedVfsDevice *dad = new DAttachedVfsDevice(path);
        if (dad->isValid()) {
            dad->detach();
        } else {
            qDebug() << "dad->isValid()" << mount->name();
        }
    }
}

const QList<QExplicitlySharedDataPointer<DGioMount> > DiskControlWidget::getVfsMountList()
{
    QList<QExplicitlySharedDataPointer<DGioMount> > result;
    const QList<QExplicitlySharedDataPointer<DGioMount> > mounts = m_vfsManager->getMounts();
    for (auto mount : mounts) {
        QExplicitlySharedDataPointer<DGioFile> file = mount->getRootFile();
        QString uriStr = file->uri();
        QUrl url(uriStr);

#ifdef QT_DEBUG
        if (!url.isValid()) {
            qWarning() << "Gio uri is not a vaild QUrl!" << uriStr;
            //qFatal("See the above warning for reason");
        }
#endif // QT_DEBUG

        if (url.scheme() == "file") continue;

        result.append(mount);
    }

    return result;
}

void DiskControlWidget::onDiskListChanged()
{
    auto addSeparateLine = [this](int width = 1){
        QFrame *line = new QFrame(this);
        line->setLineWidth(width);
        line->setFrameStyle(QFrame::HLine);
        line->setFrameShadow(QFrame::Plain);
        m_centralLayout->addWidget(line);
    };

    while (QLayoutItem *item = m_centralLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

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
    m_centralLayout->addWidget(header);
    addSeparateLine(2);

    int mountedCount = 0;

    QStringList blDevList = DDiskManager::blockDevices({});
    for (const QString &blDevStr : blDevList) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));
        if (blDev->hasFileSystem() && !blDev->mountPoints().isEmpty() && !blDev->hintSystem() && !blDev->hintIgnore()) {
            if (isProtectedDevice(blDev.data())) continue;
            mountedCount++;
            QStringList blDevStrArray = blDevStr.split(QDir::separator());
            QString tagName = blDevStrArray.isEmpty() ? "" : blDevStrArray.last();
            DAttachedUdisks2Device *dad = new DAttachedUdisks2Device(blDev.data());
            qDebug() << "create new item, tagname is" << tagName;
            DiskControlItem *item = new DiskControlItem(dad, this);
            item->setTagName(tagName);

            class ErrHandle : public ErrorHandleInfc, public QObject
            {
            public:
                explicit ErrHandle(QObject *parent): QObject(parent) {}
                virtual void onError(DAttachedDeviceInterface *device)
                {
                    DAttachedUdisks2Device *drv = dynamic_cast<DAttachedUdisks2Device *>(device);
                    if (drv) {
                        qWarning() << drv->blockDevice()->lastError().name() << device->displayName();
                        NotifyMsg(DiskControlWidget::tr("The device was not safely removed"), tr("Click \"Safely Remove\" and then disconnect it next time"));
                    }
                }
            };
            dad->setErrorHandler(new ErrHandle(item));

            m_centralLayout->addWidget(item);
            addSeparateLine(1);
        }
    }

    const QList<QExplicitlySharedDataPointer<DGioMount> > mounts = getVfsMountList();
    for (auto mount : mounts) {
        if (mount->isShadowed()) {
            continue;
        }
        QExplicitlySharedDataPointer<DGioFile> rootFile = mount->getRootFile();
        QString path = rootFile->path();
        DAttachedVfsDevice *dad = new DAttachedVfsDevice(path);
        if (dad->isValid()) {
            mountedCount++;
            DiskControlItem *item = new DiskControlItem(dad, this);
            m_centralLayout->addWidget(item);
            addSeparateLine(1);
        } else {
            delete dad;
            dad = nullptr; //指针指空 防止野指针崩溃不好找
        }
    }

    // 移除最下面的分割线
    QLayoutItem *last = m_centralLayout->takeAt(m_centralLayout->count() - 1);
    if (last) {
        delete last->widget();
        delete last;
    }

    emit diskCountChanged(mountedCount);

    const int contentHeight = mountedCount * 70 + 46;
    const int maxHeight = std::min(contentHeight, 70 * 6);

    m_centralWidget->setFixedHeight(contentHeight);
    setFixedHeight(maxHeight);

    verticalScrollBar()->setPageStep(maxHeight);
    verticalScrollBar()->setMaximum(contentHeight - maxHeight);
}

void DiskControlWidget::onDriveConnected(const QString &deviceId)
{
    QScopedPointer<DDiskDevice> diskDevice(DDiskManager::createDiskDevice(deviceId));
    if (diskDevice->removable()) {
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceAdded);
    }
}

void DiskControlWidget::onDriveDisconnected()
{
    qDebug() << "changed from drive_disconnected";
    NotifyMsg(QObject::tr("The device has been safely removed"));
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceRemoved);
    onDiskListChanged();
}

void DiskControlWidget::onMountAdded()
{
    qDebug() << "changed from mount_add";
    onDiskListChanged();
}

void DiskControlWidget::onMountRemoved(const QString &blockDevicePath, const QByteArray &mountPoint)
{
    Q_UNUSED(mountPoint);
    qDebug() << "changed from mount_remove:" << blockDevicePath;
    QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blockDevicePath));
    if (blDev) {
        QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blDev->drive()));
        if (diskDev && diskDev->removable()) {
            qDebug() << "removable device" << blockDevicePath;// << mountPoint;
            //return; // removable device emit onDiskListChanged too
        }
    }

    qDebug() << "unmounted," << mountPoint;
    onDiskListChanged();
}

void DiskControlWidget::onVolumeAdded()
{
    qDebug() << "changed from volume_add";
    onDiskListChanged();
}

void DiskControlWidget::onVolumeRemoved()
{
    qDebug() << "changed from volume_remove";
    onDiskListChanged();
}

void DiskControlWidget::onVfsMountChanged(QExplicitlySharedDataPointer<DGioMount> mount)
{
    qDebug() << "changed from VfsMount";
    QExplicitlySharedDataPointer<DGioFile> file = mount->getRootFile();
    QString uriStr = file->uri();
    QUrl url(uriStr);

#ifdef QT_DEBUG
    if (!url.isValid()) {
        qWarning() << "Gio uri is not a vaild QUrl!" << uriStr;
        //qFatal("See the above warning for reason");
    }
#endif // QT_DEBUG

    if (url.scheme() == "file") return;

    onDiskListChanged();
}

void DiskControlWidget::onBlockDeviceAdded(const QString &path)
{
    static const QString msg = "device add canceld: ";
    // 刷新一次配置信息当有新的设备接入时，保证每次都是最新的配置生效
    getGsGlobal()->reload();
    m_autoMountEnable = getGsGlobal()->value("GenericAttribute", "AutoMount", false).toBool();
    m_autoMountAndOpenEnable = getGsGlobal()->value("GenericAttribute", "AutoMountAndOpen", false).toBool();

    if (m_isInLiveSystem || !m_autoMountEnable) {
        qDebug() << msg + "isInLiveSys-" << m_isInLiveSystem << "\tautoMountEnable-" << m_autoMountEnable;
        return;
    }
    // 以前的逻辑，应该是判定当前用户是否是激活状态（登录系统状态）
    QDBusInterface loginManager("org.freedesktop.login1",
                                "/org/freedesktop/login1/user/self",
                                "org.freedesktop.login1.User",
                                QDBusConnection::systemBus());
    QVariant replay = loginManager.property(("State"));
    if (replay.isValid()) {
        QString state = replay.toString();
        if (state != "active") {
            qDebug() << msg + "user state is not active";
            return;
        }
    }

    qInfo() << "try to convert blkdev from path: " << path ;

    QScopedPointer<DBlockDevice> blkDev(DDiskManager::createBlockDevice(path));

    // 如果已经在 onDriveConnected 函数中挂载上了的设备直接忽略
    if (!blkDev || !blkDev->mountPoints().isEmpty()) return;
    // 以下皆是以前的逻辑
    if (isProtectedDevice(blkDev.data())) return;
    if (blkDev->isEncrypted()) return;
    if (blkDev->hintIgnore()) return;
    if (!blkDev->hasFileSystem()) return;

    QString mountPoint = blkDev->mount({});

    if (mountPoint.isEmpty() || blkDev->lastError().type() != QDBusError::NoError) {
        qDebug() << "auto mount error: " << blkDev->lastError().type() << blkDev->lastError().message();
        qDebug() << msg + "mount error occured";
        return;
    }

    qInfo() << "auto mount drive[ " << blkDev->drive() << " ] with path[" << path << " ] on point:" << mountPoint;

    if (m_autoMountAndOpenEnable) {
        // 不太明白为什么要保留这段代码
        if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
            QString mountUrlStr = DFMROOT_ROOT + path.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS;
            QProcess::startDetached(QStringLiteral("dde-file-manager"), {mountUrlStr});
            qDebug() << "open by dde-file-manager: " << mountUrlStr;
            return;
        }
        DDesktopServices::showFolder(QUrl::fromLocalFile(mountPoint));
    }
}

void DiskControlWidget::NotifyMsg(QString msg)
{
    DDBusSender()
    .service("org.freedesktop.Notifications")
    .path("/org/freedesktop/Notifications")
    .interface("org.freedesktop.Notifications")
    .method(QString("Notify"))
    .arg(tr("dde-file-manager"))
    .arg(static_cast<uint>(0))
    .arg(QString("media-eject"))
    .arg(msg)
    .arg(QString())
    .arg(QStringList())
    .arg(QVariantMap())
    .arg(5000).call();
}

void DiskControlWidget::NotifyMsg(QString title, QString msg)
{
    DDBusSender()
    .service("org.freedesktop.Notifications")
    .path("/org/freedesktop/Notifications")
    .interface("org.freedesktop.Notifications")
    .method(QString("Notify"))
    .arg(tr("dde-file-manager"))
    .arg(static_cast<uint>(0))
    .arg(QString("media-eject"))
    .arg(title)
    .arg(msg)
    .arg(QStringList())
    .arg(QVariantMap())
    .arg(5000).call();
}
