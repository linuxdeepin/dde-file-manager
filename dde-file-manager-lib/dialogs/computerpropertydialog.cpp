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

#include "computerpropertydialog.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"

#include "shutil/fileutils.h"

#include <DPlatformWindowHandle>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <DSysInfo>
#include <DTitlebar>
#include <QGridLayout>
#include <QPixmap>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
#include <QPainter>
#include <DGraphicsClipEffect>

#include "dbus/dbussysteminfo.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

ComputerPropertyDialog::ComputerPropertyDialog(QWidget *parent) : DDialog(parent)
{
    m_systemInfo = new DBusSystemInfo();
    initUI();
}

void ComputerPropertyDialog::initUI()
{
    QLabel *iconLabel = new QLabel(this);

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    //在wayland平台下设置固定大小，解决属性框最大化问题
    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {

        this->setFixedSize(320, 420);
    }

    QString distributerLogoPath = DSysInfo::distributionOrgLogo();
    QIcon logoIcon;
    if (!distributerLogoPath.isEmpty() && QFile::exists(distributerLogoPath)) {
        logoIcon = QIcon(distributerLogoPath);
    } else {
        logoIcon = QIcon::fromTheme("dfm_deepin_logo");
    }

    iconLabel->setPixmap(logoIcon.pixmap(152, 39));
    QLabel *nameLabel = new QLabel(tr("Computer"), this);
    auto pt = nameLabel->palette();
    pt.setColor(QPalette::Text, palette().color(QPalette::Normal, QPalette::Text));
    auto font = nameLabel->font();
    font.setBold(true);
    font.setPixelSize(17);
    nameLabel->setFont(font);
    nameLabel->setPalette(pt);

    QLabel *messageLabel = new QLabel(tr("Basic Info"), this);
    messageLabel->setPalette(pt);
    messageLabel->setFont(font);


    QGridLayout *gridLayout = new QGridLayout;
    //gridLayout->setColumnMinimumWidth(0, 100);
    gridLayout->setColumnMinimumWidth(1, 170);
    gridLayout->setSpacing(10);

    QStringList msgsTitle;
    msgsTitle << tr("Computer Name")
              << tr("Version")
              << tr("Type")
              << tr("Processor")
              << tr("Memory");

    int row = 0;
    QHash<QString, QString> datas = getMessage(msgsTitle);

    foreach (const QString &key, msgsTitle) {
        QLabel *keyLabel = new QLabel(key, this);
        QLabel *valLabel = new QLabel(datas.value(key), this);

        valLabel->setTextFormat(Qt::PlainText);
        valLabel->setWordWrap(true);

        font = keyLabel->font();
        font.setPixelSize(13);
        keyLabel->setFont(font);

        font = valLabel->font();
        font.setPixelSize(12);
        valLabel->setFont(font);

        pt = keyLabel->palette();
        pt.setColor(QPalette::Text, palette().color(QPalette::Normal, QPalette::HighlightedText));
        keyLabel->setPalette(pt);

        pt = valLabel->palette();
        pt.setColor(QPalette::Text, palette().color(QPalette::Inactive, QPalette::Text));
        valLabel->setPalette(pt);
//        keyLabel->setStyleSheet("QLabel { color: #001A2E; font-size: 13px; }");
//        valLabel->setStyleSheet("QLabel { color: #526A7F; font-size: 12px; }");

        gridLayout->addWidget(keyLabel, row, 0, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->addWidget(valLabel, row, 1, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->setRowMinimumHeight(row, valLabel->heightForWidth(gridLayout->columnMinimumWidth(1)));
        row++;
    }

    QFrame *contentFrame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(21);

    QFrame *baseInfoFrame = new QFrame;
    QString backColor = palette().color(QPalette::Base).name();
    baseInfoFrame->setStyleSheet(QString("background-color: %1; border-radius: 8px;").arg(backColor));
    QVBoxLayout *baseInfoLayout = new QVBoxLayout;
    baseInfoFrame->setLayout(baseInfoLayout);
    baseInfoLayout->addWidget(messageLabel);
    baseInfoLayout->addSpacing(10);
    baseInfoLayout->setContentsMargins(10, 5, 10, 5);
    baseInfoLayout->addLayout(gridLayout);

    mainLayout->addWidget(baseInfoFrame);

    setFixedWidth(320);
    contentFrame->setLayout(mainLayout);

    addContent(contentFrame);
}

static QString formatCap(qulonglong cap, const int size = 1024, quint8 precision = 1)
{
    static QString type[] = {" B", " KB", " MB", " GB", " TB"};

    qulonglong lc = cap;
    double dc = cap;
    double ds = size;

    for (size_t p = 0; p < sizeof(type); ++p) {
        if (cap < pow(size, p + 1) || p == sizeof(type) - 1) {
            if (!precision) {
                return QString::number(round(lc / pow(size, p))) + type[p];
            }

            return QString::number(dc / pow(ds, p), 'f', precision) + type[p];
        }
    }

    return "";
}

QHash<QString, QString> ComputerPropertyDialog::getMessage(const QStringList &data)
{
    QHash<QString, QString> datas;
    QString version;

    if (DSysInfo::isDeepin()) {
        version = DSysInfo::deepinVersion().split(" ").first() + " " + DSysInfo::deepinTypeDisplayName();
    } else {
        version = QString("%1 %2").arg(DSysInfo::productTypeString())
                  .arg(DSysInfo::productVersion());
    }

//    qDebug() << "1111" << DSysInfo::deepinEdition() << "222" << DSysInfo::deepinCopyright() << "333" << DSysInfo::deepinTypeDisplayName()
//             << "444" << DSysInfo::productTypeString() << "555" << DSysInfo::productVersion() << "666" << DSysInfo::operatingSystemName();
    //部分数据优先从dbus读取，如果dbus没有，则从dtk读数据

    QDBusInterface *t_systemInfo = new QDBusInterface("com.deepin.system.SystemInfo",
                                                      "/com/deepin/system/SystemInfo",
                                                      "com.deepin.system.SystemInfo",
                                                      QDBusConnection::systemBus(), this);
    QString memoryInstallStr;
    QString memoryStr;
    QString processor;
    QString systemType;
    if (!t_systemInfo->isValid()) {
        memoryInstallStr = formatCap(DSysInfo::memoryInstalledSize(), 1024, 0);
        memoryStr = formatCap(DSysInfo::memoryTotalSize());

        processor = m_systemInfo->processor();
        systemType = QString::number(m_systemInfo->systemType());
        version = m_systemInfo->version();
    } else {
//        memoryInstallStr = QString::number(static_cast<double>(m_systemInfo->memoryCap()) / (1000 * 1000 * 1000), 'f', 0);
//        memoryStr = QString::number(static_cast<double>(m_systemInfo->memoryCap()) / (1024 * 1024 * 1024), 'f', 1);

        memoryInstallStr = formatCap(t_systemInfo->property("MemorySize").toULongLong(), 1024, 0);
        memoryStr = formatCap(DSysInfo::memoryTotalSize());

        processor = m_systemInfo->processor();
        systemType = QString::number(m_systemInfo->systemType());
        version = m_systemInfo->version();
    }

    version = version.replace("Personal", "Home");

    datas.insert(data.at(0), DSysInfo::computerName());
    datas.insert(data.at(1), version);
    datas.insert(data.at(2), QString::number(m_systemInfo->systemType()) + tr("Bit"));
    datas.insert(data.at(3), processor);
    datas.insert(data.at(4), memoryInstallStr + "(" +  memoryStr + ' ' + tr("Available") + ")");

    return datas;
}
