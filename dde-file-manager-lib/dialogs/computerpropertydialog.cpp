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

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

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
    nameLabel->setStyleSheet("QLabel { font-size: 13px; }");

    QLabel *lineLabel = new QLabel(this);
    lineLabel->setObjectName("Line");
    lineLabel->setFixedSize(300, 2);
    lineLabel->setStyleSheet("QLabel#Line{"
                             "border: none;"
                             "background-color: #f0f0f0;"
                             "}");

    QLabel *messageLabel = new QLabel(tr("Basic Info"), this);
    messageLabel->setStyleSheet("QLabel { font-size: 13px; }");

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setColumnMinimumWidth(0, 100);
    gridLayout->setColumnMinimumWidth(1, 170);
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
        QLabel *valLabel = new QLabel(datas.value(key), this);

        valLabel->setTextFormat(Qt::PlainText);
        valLabel->setWordWrap(true);

        keyLabel->setStyleSheet("QLabel { color: #777777; font-size: 12px; }");
        valLabel->setStyleSheet("QLabel { font-size: 12px; }");

        gridLayout->addWidget(keyLabel, row, 0, Qt::AlignRight | Qt::AlignTop);
        gridLayout->addWidget(valLabel, row, 1, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->setRowMinimumHeight(row, valLabel->heightForWidth(gridLayout->columnMinimumWidth(1)));
        row++;
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
}

QHash<QString, QString> ComputerPropertyDialog::getMessage(const QStringList &data)
{
    QHash<QString, QString> datas;
    QString version;

    if (DSysInfo::isDeepin()) {
        version = QString("%1 %2").arg(DSysInfo::productVersion())
                                  .arg(DSysInfo::deepinTypeDisplayName());
    } else {
        version = QString("%1 %2").arg(DSysInfo::productTypeString())
                                  .arg(DSysInfo::productVersion());
    }

    datas.insert(data.at(0), DSysInfo::computerName());
    datas.insert(data.at(1), version);
    datas.insert(data.at(2), QString::number(QSysInfo::WordSize) + tr("Bit"));
    datas.insert(data.at(3), QString("%1 x %2").arg(DSysInfo::cpuModelName())
                                               .arg(QThread::idealThreadCount()));
    datas.insert(data.at(4), FileUtils::formatSize(DSysInfo::memoryTotalSize()));
    datas.insert(data.at(5), FileUtils::formatSize(DSysInfo::systemDiskSize()));

    return datas;
}
