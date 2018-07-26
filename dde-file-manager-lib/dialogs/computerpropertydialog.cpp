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
#include "dfmdiskmanager.h"
#include "dfmblockdevice.h"
#include "dfmdiskdevice.h"

#include "shutil/fileutils.h"

#include <DPlatformWindowHandle>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <DTitlebar>
#include <QGridLayout>
#include <QPixmap>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>

DFM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

ComputerPropertyDialog::ComputerPropertyDialog(QWidget *parent) : DDialog(parent)
{
    initUI();
}

void ComputerPropertyDialog::initUI()
{
    QLabel *iconLabel = new QLabel(this);
    QIcon logoIcon;
    logoIcon.addFile(":/images/dialogs/images/deepin_logo.png");
    logoIcon.addFile(":/images/dialogs/images/deepin_logo@2x.png");

    iconLabel->setPixmap(logoIcon.pixmap(152, 39));
    QLabel *nameLabel = new QLabel(tr("Computer"), this);
    nameLabel->setObjectName("NameLabel");

    QLabel *lineLabel = new QLabel(this);
    lineLabel->setObjectName("Line");
    lineLabel->setFixedSize(300, 2);
    lineLabel->setStyleSheet("QLabel#Line{"
                             "border: none;"
                             "background-color: #f0f0f0;"
                             "}");

    QLabel *messageLabel = new QLabel(tr("Basic Info"), this);
    messageLabel->setObjectName("NameLabel");

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setColumnMinimumWidth(0, 100);
    gridLayout->setColumnMinimumWidth(1, 200);
    gridLayout->setSpacing(10);

    QStringList msgsTitle;
    msgsTitle << tr("Computer Name")
              << tr("Version")
              << tr("Type")
              << tr("Processor")
              << tr("Memory")
              << tr("Disk");

    int row = 0;
    QHash<QString, QString> datas = getMessage(msgsTitle);

    foreach (const QString &key, msgsTitle) {
        QLabel *keyLabel = new QLabel(key, this);
        keyLabel->setObjectName("KeyLabel");
        QLabel *valLabel = new QLabel(datas.value(key), this);
        valLabel->setTextFormat(Qt::PlainText);
        valLabel->setObjectName("ValLabel");
        valLabel->setWordWrap(true);

        gridLayout->addWidget(keyLabel, row, 0, Qt::AlignRight | Qt::AlignTop);
        gridLayout->addWidget(valLabel, row, 1, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->setRowMinimumHeight(row, 12);
        row ++;
    }

    QFrame *contentFrame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(60);
    mainLayout->addWidget(nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(lineLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(5);
    QHBoxLayout *messageLayout = new QHBoxLayout;
    messageLayout->setSpacing(0);
    messageLayout->addSpacing(30);
    messageLayout->addWidget(messageLabel);
    mainLayout->addLayout(messageLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch(1);

    setFixedSize(320, 460);
    contentFrame->setLayout(mainLayout);

    addContent(contentFrame);

    setStyleSheet("QLabel#NameLabel{"
                  "font-size: 12px;"
                  "}"
                  "QLabel#KeyLabel{"
                  "font-size: 11px;"
                  "color: #777777;"
                  "}"
                  "QLabel#ValLabel{"
                  "font-size:11px;"
                  "}");
}

QHash<QString, QString> ComputerPropertyDialog::getMessage(const QStringList &data)
{
    QHash<QString, QString> datas;
    datas.insert(data.at(0), getComputerName());
    datas.insert(data.at(1), getVersion());
    datas.insert(data.at(2), getArch());
    datas.insert(data.at(3), getProcessor());
    datas.insert(data.at(4), getMemory());
    datas.insert(data.at(5), getDisk());
    return datas;
}

QString ComputerPropertyDialog::getComputerName()
{
    QString cmd = "hostname";
    QStringList args;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    QString result = p.readLine();
    return result.trimmed();
}

QString ComputerPropertyDialog::getVersion()
{
    QString cmd = "cat";
    QStringList args;
    args << "/etc/deepin-version";
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);

    QMap<QString, QString> msgMap;
    while (p.canReadLine()) {
        QString str = p.readLine();
        if (!str.contains("=")) {
            continue;
        }

        str = str.trimmed();
        QStringList vals = str.split("=");

        msgMap.insert(vals.first(), vals.last());
    }

    QString result;

    QString companyName = "deepin";
    QString version = msgMap.value("Version");
    QString type;

    QString local = QLocale::system().name();
    if (msgMap.contains(QString("Type[%1]").arg(local))) {
        type = msgMap.value(QString("Type[%1]").arg(local));
    } else {
        type = msgMap.value("Type");
    }

    result = result + QString("%1 %2 %3").arg(companyName, version, type);
    return result.trimmed();
}

QString ComputerPropertyDialog::getArch()
{
    QString cmd = "arch";
    QStringList args;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    QString result = p.readLine();
    result = result.trimmed();
    return (result.right(2) + tr("Bit"));
}

QString ComputerPropertyDialog::getProcessor()
{
    QString cmd = "lscpu";
    QStringList args;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    QString result;

    QMap<QString, QString> msgMap;
    while (p.canReadLine()) {
        QString str = p.readLine();
        QStringList vals = str.split(":");
        if (vals.first() == "Model name") {
            QString val = vals.last();
            msgMap.insert(vals.first(), val.trimmed());
        }
    }

    QString modelName = msgMap.value("Model name");

#ifdef SW_CPUINFO
    QString hz = getSWProcessorHZ();
    modelName = QString("%1 %2").arg(modelName, hz);
#endif

    QString num;
    cmd = "nproc";
    QProcess nproc;
    nproc.start(cmd, args);
    nproc.waitForFinished(-1);
    num = nproc.readLine();

    result = QString("%1 x %2").arg(modelName, num);

    return result.trimmed();
}

QString ComputerPropertyDialog::getSWProcessorHZ()
{
    QString cmd = "cat";
    QStringList args;
    args << "/proc/cpuinfo";
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);

    QString result;
    while (p.canReadLine()) {
        QString str = p.readLine();
        QStringList vals = str.split(":");
        if (vals.first().startsWith("CPU frequency [MHz]")) {
            result = vals.last().trimmed();
            break;
        }
    }
    double hz = result.toDouble() / 1000.0;
    return QString("%1 GHz").arg(QString::number(hz, 'f', 2));
}

// blumia: it should be GiB...
static QString formatCap(qulonglong cap, const int size = 1024)
{
    static QString type[] = {" B", " KB", " MB", " GB", " TB"};

    if (cap < qulonglong(size)) {
        return QString::number(cap) + type[0];
    }
    if (cap < qulonglong(size) * size) {
        return QString::number(double(cap) / size, 'f', 2) + type[1];
    }
    if (cap < qulonglong(size) * size * size) {
        return QString::number(double(cap) / size / size, 'f', 2) + type[2];
    }
    if (cap < qulonglong(size) * size * size * size) {
        return QString::number(double(cap) / size / size / size, 'f', 2) + type[3];
    }
    return QString::number(double(cap) / size / size / size / size, 'f', 2) + type[4];
}

QString ComputerPropertyDialog::getMemory()
{
    QFile meminfo("/proc/meminfo");
    Q_ASSERT(meminfo.open(QIODevice::ReadOnly));
    qulonglong mem_kB = 0;
    char singleLine[161] {};
    while (meminfo.readLine(singleLine, 160) > 0) {
        QString line(singleLine);
        if (line.contains(QStringLiteral("MemTotal"))) {
            QRegularExpression re("(\\d+)");
            QRegularExpressionMatch match;
            match = re.match(line);
            mem_kB = match.captured(0).toULongLong();
            break;
        }
    }

    if (mem_kB > 0) {
        return formatCap(mem_kB * 1024);
    }

    QString cmd = "free";
    QProcess p;
    p.start(cmd, (QStringList() << "-h"));
    p.waitForFinished(-1);
    p.readLine();
    QString str = p.readLine();
    QStringList datas = str.split(" ");
    for (int i = 0; i < datas.length(); i++) {
        if (datas.at(i) == "") {
            datas.removeAt(i);
            i--;
        }
    }
    QString result = datas.at(1);
    return result.trimmed();
}

QString ComputerPropertyDialog::getDisk()
{
    DFMDiskManager manager;

    for (const QString &device_path : manager.blockDevices()) {
        QScopedPointer<DFMBlockDevice> block_device(manager.createBlockDevice(device_path));

        for (const QByteArray &mount_poion : block_device->mountPoints()) {
            if (QFile::encodeName(mount_poion) == QDir::rootPath()) {
                QScopedPointer<DFMDiskDevice> disk_device(manager.createDiskDevice(block_device->drive()));

                return FileUtils::formatSize(disk_device->size());
            }
        }
    }

    return QString();
}
