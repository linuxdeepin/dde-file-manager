// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicebasicwidget.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

static constexpr int kMaximumHeight { 31 };

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;
DeviceBasicWidget::DeviceBasicWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    initUI();
    fileCalculationUtils = new FileStatisticsJob;
    connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &DeviceBasicWidget::slotFileDirSizeChange);
}

DeviceBasicWidget::~DeviceBasicWidget()
{
    fileCalculationUtils->stop();
    fileCalculationUtils->deleteLater();
}

void DeviceBasicWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    setTitle(QString(tr("Basic info")));

    setExpand(true);

    deviceInfoFrame = new QFrame(this);

    deviceType = new KeyValueLabel(this);
    deviceType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    deviceType->setLeftValue(tr("Device type"));
    deviceTotalSize = new KeyValueLabel(this);
    deviceTotalSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    deviceTotalSize->setLeftValue(tr("Total space"));
    fileSystem = new KeyValueLabel(this);
    fileSystem->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSystem->setLeftValue(tr("File system"));
    fileCount = new KeyValueLabel(this);
    fileCount->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCount->setLeftValue(tr("Contains"));
    fileCount->rightWidget()->setMaximumHeight(kMaximumHeight);
    freeSize = new KeyValueLabel(this);
    freeSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    freeSize->setLeftValue(tr("Free space"));
    // 1602914
    QGridLayout *glayout = new QGridLayout;
    glayout->setContentsMargins(15, 15, 5, 10);
    glayout->setSpacing(16);
    glayout->addWidget(deviceType, 0, 0, 1, 6);
    glayout->addWidget(deviceTotalSize, 1, 0, 1, 6);
    glayout->addWidget(fileSystem, 2, 0, 1, 6);
    glayout->addWidget(fileCount, 3, 0, 1, 6);
    glayout->addWidget(freeSize, 4, 0, 1, 6);
    glayout->setColumnStretch(0, 1);
    deviceInfoFrame->setLayout(glayout);

    setContent(deviceInfoFrame);
}

void DeviceBasicWidget::selectFileUrl(const QUrl &url)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    fileCount->setRightValue(QString::number(info->countChildFile()));
    fileCount->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
}

void DeviceBasicWidget::selectFileInfo(const DeviceInfo &info)
{
    deviceType->setRightValue(info.deviceType);
    deviceType->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);

    QString sizeTotalStr = UniversalUtils::sizeFormat(info.totalCapacity, 1);
    deviceTotalSize->setRightValue(sizeTotalStr);
    deviceTotalSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);

    if (info.fileSystem.isEmpty())
        fileSystem->hide();
    fileSystem->setRightValue(info.fileSystem);
    fileSystem->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);

    QString sizeFreeStr = UniversalUtils::sizeFormat(info.availableSpace, 1);
    if (info.mountPoint.isEmpty())
        sizeFreeStr = UniversalUtils::sizeFormat(info.totalCapacity, 1);
    freeSize->setRightValue(sizeFreeStr);
    freeSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);

    fileCalculationUtils->setFileHints(FileStatisticsJob::kExcludeSourceFile);
    fileCalculationUtils->start(QList<QUrl>() << info.mountPoint);
}

void DeviceBasicWidget::slotFileDirSizeChange(qint64 size, int filesCount, int directoryCount)
{
    Q_UNUSED(size)
    int cnt = filesCount + directoryCount;
    QString txt = cnt > 1 ? tr("%1 items") : tr("%1 item");
    fileCount->setRightValue(txt.arg(cnt), Qt::ElideNone, Qt::AlignVCenter, false);
}
