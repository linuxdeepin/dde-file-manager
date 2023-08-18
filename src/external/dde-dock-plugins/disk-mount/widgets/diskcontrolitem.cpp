// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskcontrolitem.h"
#include "diskcontrolwidget.h"
#include "utils/dockutils.h"

#include <DGuiApplicationHelper>
#include <DDialog>
#include <DDesktopServices>
#include <QVBoxLayout>
#include <QGSettings>
#include <QFile>
#include <QStandardPaths>
#include <QProcess>

/*!
 * \class DiskControlItem
 *
 * \brief DiskControlItem is an item on the DiskControlWidget
 */

DWIDGET_USE_NAMESPACE

DiskControlItem::DiskControlItem(QSharedPointer<DAttachedDevice> attachedDevicePtr, QWidget *parent)
    : QFrame(parent),
      unknowIcon(":/icons/resources/unknown.svg"),
      diskIcon(new QPushButton(this)),
      diskName(new QLabel),
      diskCapacity(new QLabel),
      capacityValueBar(new QProgressBar),
      attachedDev(attachedDevicePtr),
      ejectButton(new DIconButton(this)),
      par(qobject_cast<DiskControlWidget *>(parent))
{
    setObjectName("DiskItem");

    initializeUi();
    initConnection();
}

void DiskControlItem::detachDevice()
{
    // eject all partions of device when a device eject clicked
    attachedDev->detach();
}

void DiskControlItem::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    QGSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");
    if (!gsettings.get("filemanagerIntegration").toBool()) {
        qWarning() << "GSetting `filemanager-integration` is false";
        return;
    }

    QUrl &&mountPoint = QUrl(attachedDev->mountpointUrl());
    QUrl &&url = QUrl(attachedDev->accessPointUrl());

    qInfo() << "Open " << url;

    // 光盘文件系统剥离 RockRidge 后，udisks 的默认挂载权限为 500，为遵从 linux 权限限制，在这里添加访问目录的权限校验
    QFile f(mountPoint.path());
    if (url.scheme() == "burn" && f.exists() && !f.permissions().testFlag(QFile::ExeUser)) {
        DDialog *d = new DDialog(QObject::tr("Access denied"), QObject::tr("You do not have permission to access this folder"));
        d->setAttribute(Qt::WA_DeleteOnClose);
        Qt::WindowFlags flags = d->windowFlags();
        d->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        d->setIcon(QIcon::fromTheme("dialog-error"));
        d->addButton(QObject::tr("Confirm", "button"), true, DDialog::ButtonRecommend);
        d->setMaximumWidth(640);
        d->show();
        return;
    }

    if (url.scheme() == "burn") {
        // 1. 当前熊默认文件管理器为 dde-file-manager 时，使用它打开光盘
        // 2. 默认文件管理器为其他时，依然采用打开挂载点的方式
        if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
            QString &&path = url.path();
            QString &&opticalPath = QString("burn://%1").arg(path);
            qDebug() << "open optical path =>" << opticalPath;
            QProcess::startDetached(QStringLiteral("dde-file-manager"), { opticalPath });
        } else {
            url = QUrl(attachedDev->mountpointUrl());
            DDesktopServices::showFolder(url);
        }
    } else {
        DDesktopServices::showFolder(url);
    }
}

void DiskControlItem::showEvent(QShowEvent *e)
{
    attachedDev->query();
    diskName->setText(attachedDev->displayName());
    QString &&name = attachedDev->displayName();
    auto &&f = diskName->font();
    QFontMetrics fm(f);
    QString &&elideText = fm.elidedText(name, Qt::ElideRight, diskName->width());
    diskName->setText(elideText);

    if (attachedDev->deviceUsageValid()) {
        QPair<quint64, quint64> &&freeAndTotal = attachedDev->deviceUsage();
        quint64 bytesFree = freeAndTotal.first;
        quint64 bytesTotal = freeAndTotal.second;
        quint64 bytesUsage = bytesTotal > bytesFree ? (bytesTotal - bytesFree) : 0;

        diskCapacity->setText(QString("%1 / %2")
                                      .arg(size_format::formatDiskSize(bytesUsage))
                                      .arg(size_format::formatDiskSize(bytesTotal)));

        if (bytesTotal > 0) {
            capacityValueBar->setValue(static_cast<int>(100 * (bytesTotal - bytesFree) / bytesTotal));
        }
    }

    QFrame::showEvent(e);
}

void DiskControlItem::initializeUi()
{
    diskName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    diskName->setTextFormat(Qt::PlainText);

    int colorValue = Dtk::Gui::DGuiApplicationHelper::instance()->themeType()
                    == Dtk::Gui::DGuiApplicationHelper::LightType
            ? 0
            : 1;

    QFont f = diskName->font();
    f.setPixelSize(14);
    f.setWeight(QFont::Medium);
    diskName->setFont(f);
    QPalette pal = diskName->palette();
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorValue, colorValue, colorValue, 0.8));
    diskName->setPalette(pal);

    f = diskCapacity->font();
    f.setPixelSize(12);
    f.setWeight(QFont::Normal);
    diskCapacity->setFont(f);
    pal = diskCapacity->palette();
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorValue, colorValue, colorValue, 0.6));
    diskCapacity->setPalette(pal);

    capacityValueBar->setTextVisible(false);
    capacityValueBar->setFixedHeight(2);

    ejectButton->setFixedSize(20, 20);
    ejectButton->setIconSize({ 20, 20 });
    ejectButton->setFlat(true);

    QVBoxLayout *leftLay = new QVBoxLayout;
    leftLay->addWidget(diskIcon);
    leftLay->setContentsMargins(10, 8, 0, 8);
    leftLay->setSpacing(0);

    QWidget *info = new QWidget(this);
    QVBoxLayout *centLay = new QVBoxLayout;
    QVBoxLayout *subCentLay = new QVBoxLayout;
    subCentLay->setSpacing(2);
    subCentLay->setContentsMargins(0, 0, 0, 0);
    subCentLay->addWidget(diskName);
    subCentLay->addWidget(diskCapacity);
    centLay->addItem(subCentLay);
    centLay->addWidget(capacityValueBar);
    info->setLayout(centLay);
    centLay->setSpacing(5);
    centLay->setContentsMargins(10, 11, 0, 10);

    QVBoxLayout *rigtLay = new QVBoxLayout;
    rigtLay->addWidget(ejectButton);
    rigtLay->setContentsMargins(19, 22, 12, 22);

    QHBoxLayout *mainLay = new QHBoxLayout;
    mainLay->addLayout(leftLay);
    mainLay->addWidget(info);
    mainLay->addLayout(rigtLay);
    mainLay->setContentsMargins(10, 8, 8, 12);
    mainLay->setMargin(0);
    mainLay->setSpacing(0);
    setLayout(mainLay);

    // Note: Setting `DisableNonRemovableDeviceUnmount` not exist today

    diskIcon->setFlat(true);
    diskIcon->setIcon(QIcon::fromTheme(attachedDev->iconName(), unknowIcon));
    diskIcon->setIconSize(QSize(48, 48));
    diskIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    diskIcon->setStyleSheet("padding: 0;");
    diskName->setText(QStringLiteral("OwO"));   // blumia: correct text should be set in DiskControlItem::showEvent()
    capacityValueBar->setMinimum(0);
    capacityValueBar->setMaximum(100);

    refreshIcon();
}

void DiskControlItem::initConnection()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DiskControlItem::refreshIcon);
    connect(ejectButton, &DIconButton::clicked, this, &DiskControlItem::detachDevice);
}

void DiskControlItem::refreshIcon()
{
    ejectButton->setIcon(QIcon::fromTheme("dfm_unmount"));
}
