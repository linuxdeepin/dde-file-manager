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

#include "diskcontrolitem.h"
#include "dattachedudisks2device.h"

#include "dfmglobal.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QtMath>
#include <QApplication>
#include <QDebug>
#include <QStorageInfo>
#include <QProcess>
#include <DDialog>

#include <dgiosettings.h>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <dfmsettings.h>
#include <DDesktopServices>
#include <DGuiApplicationHelper>
#include <DIconButton>

//fix: 设置光盘容量属性
#include <dfmstandardpaths.h>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFile>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

DFM_USE_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(DFMSettings, gsGlobal, ("deepin/dde-file-manager", DFMSettings::GenericConfig))

// it takes the ownership of \a attachedDevicePtr.
DiskControlItem::DiskControlItem(DAttachedDeviceInterface *attachedDevicePtr, QWidget *parent)
    : QFrame(parent),

      m_unknowIcon(":/icons/resources/unknown.svg"),

      m_diskIcon(new QPushButton(this)),
      m_diskName(new QLabel),
      m_diskCapacity(new QLabel),
      m_capacityValueBar(new QProgressBar),
      m_unmountButton(new DIconButton(this))
{
    int colorValue = Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::LightType
            ? 0
            : 1;

    attachedDevice.reset(attachedDevicePtr);

    m_diskName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_diskName->setTextFormat(Qt::PlainText);

    QFont f = m_diskName->font();
    f.setPixelSize(14);
    f.setWeight(QFont::Medium);
    m_diskName->setFont(f);
    QPalette pal = m_diskName->palette();
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorValue, colorValue, colorValue, 0.8));
    m_diskName->setPalette(pal);

    f = m_diskCapacity->font();
    f.setPixelSize(12);
    f.setWeight(QFont::Normal);
    m_diskCapacity->setFont(f);
    pal = m_diskCapacity->palette();
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorValue, colorValue, colorValue, 0.6));
    m_diskCapacity->setPalette(pal);

    m_capacityValueBar->setTextVisible(false);
    m_capacityValueBar->setFixedHeight(2);

    m_unmountButton->setFixedSize(20, 20);
    m_unmountButton->setIconSize({20, 20});
    m_unmountButton->setFlat(true);

    QVBoxLayout *leftLay = new QVBoxLayout;
    leftLay->addWidget(m_diskIcon);
    leftLay->setContentsMargins(10, 8, 0, 8);
    leftLay->setSpacing(0);

    QWidget *info = new QWidget(this);
    QVBoxLayout *centLay = new QVBoxLayout;
    QVBoxLayout *subCentLay = new QVBoxLayout;
    subCentLay->setSpacing(2);
    subCentLay->setContentsMargins(0, 0, 0, 0);
    subCentLay->addWidget(m_diskName);
    subCentLay->addWidget(m_diskCapacity);
    centLay->addItem(subCentLay);
    centLay->addWidget(m_capacityValueBar);
    info->setLayout(centLay);
    centLay->setSpacing(5);
    centLay->setContentsMargins(10, 11, 0, 10);


    QVBoxLayout *rigtLay = new QVBoxLayout;
    rigtLay->addWidget(m_unmountButton);
    rigtLay->setContentsMargins(19, 22, 12, 22);

    QHBoxLayout *mainLay = new QHBoxLayout;
    mainLay->addLayout(leftLay);
    mainLay->addWidget(info);
    mainLay->addLayout(rigtLay);
    mainLay->setContentsMargins(10, 8, 8, 12);
    mainLay->setMargin(0);
    mainLay->setSpacing(0);

    setLayout(mainLay);
    setObjectName("DiskItem");

    connect(m_unmountButton, &DIconButton::clicked, this, [this]() {
        emit umountClicked(this);
    });

    if (gsGlobal->value("GenericAttribute", "DisableNonRemovableDeviceUnmount", false).toBool() && !attachedDevice->detachable()) {
        m_unmountButton->hide();
    }

    m_diskIcon->setFlat(true);
    m_diskIcon->setIcon(QIcon::fromTheme(attachedDevice->iconName(), m_unknowIcon));
    m_diskIcon->setIconSize(QSize(48, 48));
    m_diskIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_diskIcon->setStyleSheet("padding: 0;");
    m_diskName->setText(QStringLiteral("OwO")); // blumia: correct text should be set in DiskControlItem::showEvent()
    m_capacityValueBar->setMinimum(0);
    m_capacityValueBar->setMaximum(100);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DiskControlItem::refreshIcon);
    refreshIcon();
}

DiskControlItem::~DiskControlItem()
{
}

void DiskControlItem::refreshIcon()
{
    m_unmountButton->setIcon(QIcon::fromTheme("dfm_unmount"));
}

void DiskControlItem::detachDevice()
{
    attachedDevice->detach();
}

QString DiskControlItem::driveName()
{
    DAttachedUdisks2Device *device = dynamic_cast<DAttachedUdisks2Device*>(attachedDevice.data());
    if (device && device->blockDevice())
        return device->blockDevice()->drive();
    return "";
}

QString DiskControlItem::tagName() const
{
    return m_tagName;
}

void DiskControlItem::setTagName(const QString &tagName)
{
    m_tagName = tagName;
}

QUrl DiskControlItem::mountPointUrl()
{
    return attachedDevice->mountpointUrl();
}

QString DiskControlItem::sizeString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

QString DiskControlItem::formatDiskSize(const quint64 num)
{
    QStringList list {" B", " KB", " MB", " GB", " TB"};
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

void DiskControlItem::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);

    DGioSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");
    if (gsettings.value("filemanager-integration").toBool()) {
        // 光盘文件系统剥离 RockRidge 后，udisks 的默认挂载权限为 500，为遵从 linux 权限限制，在这里添加访问目录的权限校验
        DUrl mountPoint = DUrl(attachedDevice->mountpointUrl());
        QFile f(mountPoint.path());
        if (f.exists() && !f.permissions().testFlag(QFile::ExeUser)) {
            DDialog *d = new DDialog(QObject::tr("Access denied"), QObject::tr("You do not have permission to access this folder"));
            d->setAttribute(Qt::WA_DeleteOnClose);
            Qt::WindowFlags flags = d->windowFlags();
            d->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
            d->setIcon(QIcon::fromTheme("dialog-error"));
            d->addButton(QObject::tr("Confirm","button"), true, DDialog::ButtonRecommend);
            d->setMaximumWidth(640);
            d->show();
            return;
        }

        DUrl url = DUrl(attachedDevice->accessPointUrl());
        if (url.scheme() == BURN_SCHEME) {
            // 1. 当前熊默认文件管理器为 dde-file-manager 时，使用它打开光盘
            // 2. 默认文件管理器为其他时，依然采用打开挂载点的方式
            if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
                QString &&path = url.path();
                QString opticalPath = QString("burn://%1").arg(path);
                qDebug() << "open optical path =>" << opticalPath;
                QProcess::startDetached(QStringLiteral("dde-file-manager"), {opticalPath});
            } else {
                url = DUrl(attachedDevice->mountpointUrl());
                DDesktopServices::showFolder(url);
            }
        } else {
            DDesktopServices::showFolder(url);
        }
    }
}

void DiskControlItem::showEvent(QShowEvent *e)
{
    m_diskName->setText(attachedDevice->displayName());
    QString name = attachedDevice->displayName();
    QFont f = m_diskName->font();
    QFontMetrics fm(f);
    QString elideText = fm.elidedText(name, Qt::ElideRight, m_diskName->width());
    m_diskName->setText(elideText);

    if (attachedDevice->deviceUsageValid()) {
        QString iconName = attachedDevice->iconName();
        QPair<quint64, quint64> freeAndTotal = attachedDevice->deviceUsage();
        quint64 bytesFree = freeAndTotal.first;
        quint64 bytesTotal = freeAndTotal.second;

        if (iconName.simplified().toLower() == "media-optical") { // 光驱无法读取容量
            //fix: 设置光盘容量属性
            quint64 burnCapacityTotalSize =  0;
            quint64 burnCapacityUsedSize =  0;
            int burnStatus = 0;//光盘容量状态：0,光驱弹出状态 1,光驱弹入处于添加未挂载状态 2,光驱弹入处于添加后并挂载的状态
            int burnExt = 0;
            QFile burnCapacityFile(QString("%1/.config/deepin/dde-file-manager.json").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
            if (!burnCapacityFile.open(QIODevice::ReadOnly)) {
                qDebug() << "Couldn't open dde-file-manager.json burnCapacityFile!";
                return;
            }
            QByteArray burnCapacityData = burnCapacityFile.readAll();
            burnCapacityFile.close();

            QJsonParseError parseJsonErr;
            QJsonDocument jsonDoc(QJsonDocument::fromJson(burnCapacityData, &parseJsonErr));
            if (!(parseJsonErr.error == QJsonParseError::NoError)) {
                qDebug() << "decode json file error！";
                return;
            }
            QJsonObject tempBurnObjs = jsonDoc.object();
            qDebug() << "tempBurnObjs==" << tempBurnObjs;
            if (tempBurnObjs.contains(QStringLiteral("BurnCapacityAttribute"))) {
                QJsonValue jsonBurnValueList = tempBurnObjs.value(QStringLiteral("BurnCapacityAttribute"));
                QJsonObject tagItem = jsonBurnValueList.toObject();
                if (tagItem.contains(m_tagName)) {
                    QJsonObject burnItem = tagItem[m_tagName].toObject();
                    burnCapacityTotalSize =  static_cast<quint64>(burnItem["BurnCapacityTotalSize"].toDouble());
                    burnCapacityUsedSize =  static_cast<quint64>(burnItem["BurnCapacityUsedSize"].toDouble());
                    burnStatus = burnItem["BurnCapacityStatus"].toInt();
                    burnExt = burnItem["BurnCapacityExt"].toInt();
                }
            }
            Q_UNUSED(burnExt)//解决未用警告
            bytesFree = burnCapacityTotalSize - burnCapacityUsedSize;
            bytesTotal = burnCapacityTotalSize;
            if (burnStatus == 2) {
                m_diskCapacity->setText(QString("%1 / %2")
                                        .arg(formatDiskSize(burnCapacityUsedSize))
                                        .arg(formatDiskSize(burnCapacityTotalSize)));
            } else {
                m_diskCapacity->setText(tr("Unknown"));
            }
        } else {
            m_diskCapacity->setText(QString("%1 / %2")
                                    .arg(formatDiskSize(bytesTotal - bytesFree))
                                    .arg(formatDiskSize(bytesTotal)));
        }

        if (bytesTotal > 0) {
            m_capacityValueBar->setValue(static_cast<int>(100 * (bytesTotal - bytesFree) / bytesTotal));
        }
    }

    QFrame::showEvent(e);
}

DFMSettings *getGsGlobal()
{
    return gsGlobal;
}
