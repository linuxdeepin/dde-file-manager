/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "diskcontrolitem.h"
#include "sizeformathelper.h"

#include "dfm-base/dfm_global_defines.h"

#include <DGuiApplicationHelper>
#include <DDialog>
#include <DDesktopServices>
#include <QVBoxLayout>
#include <QGSettings>
#include <QFile>
#include <QStandardPaths>
#include <QProcess>

QString SizeFormatHelper::formatDiskSize(const quint64 num)
{
    QStringList list { " B", " KB", " MB", " GB", " TB" };
    qreal fileSize(num);

    QStringListIterator i(list);
    QString unit = i.next();

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }

    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', 1)), unit);
}

QString SizeFormatHelper::sizeString(const QString &str)
{
    int beginPos = str.indexOf('.');

    if (beginPos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > beginPos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

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
      ejectButton(new DIconButton(this))
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

    // 光盘文件系统剥离 RockRidge 后，udisks 的默认挂载权限为 500，为遵从 linux 权限限制，在这里添加访问目录的权限校验
    QFile f(mountPoint.path());
    if (f.exists() && !f.permissions().testFlag(QFile::ExeUser)) {
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

    QUrl &&url = QUrl(attachedDev->accessPointUrl());
    if (url.scheme() == DFMBASE_NAMESPACE::Global::kBurn) {
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
                                      .arg(SizeFormatHelper::formatDiskSize(bytesUsage))
                                      .arg(SizeFormatHelper::formatDiskSize(bytesTotal)));

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
