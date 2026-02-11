// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basicwidget.h"
#include "utils/vaultdefine.h"
#include "utils/vaulthelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/settings.h>

#include <QFileInfo>
#include <QDateTime>

static constexpr int kMaximumHeight { 31 };

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

BasicWidget::BasicWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    fmDebug() << "Vault: Creating basic property widget";
    initUI();
    fileCalculationUtils = new FileStatisticsJob;
    connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &BasicWidget::slotFileCountAndSizeChange);
    fileCalculationUtils->setFileHints(FileStatisticsJob::FileHint::kNoFollowSymlink | FileStatisticsJob::FileHint::kDontSizeInfoPointer);
}

BasicWidget::~BasicWidget()
{
    fmDebug() << "Vault: Destroying basic property widget";
    fileCalculationUtils->stop();
    fileCalculationUtils->deleteLater();
}

void BasicWidget::initUI()
{
    fmDebug() << "Vault: Initializing basic property widget UI";
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    setTitle(QString(tr("Basic info")));

    setExpand(true);

    QFrame *frame = new QFrame(this);

    fileSize = new KeyValueLabel(frame);
    fileSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSize->setLeftValue(tr("Size"));
    fileSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
    fileSize->rightWidget()->setMaximumHeight(kMaximumHeight);

    fileCount = new KeyValueLabel(frame);
    fileCount->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCount->setLeftValue(tr("Contains"));
    fileCount->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
    fileCount->rightWidget()->setMaximumHeight(kMaximumHeight);

    fileType = new KeyValueLabel(frame);
    fileType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileType->setLeftValue(tr("Type"));
    fileType->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
    fileType->rightWidget()->setMaximumHeight(kMaximumHeight);

    filePosition = new KeyValueLabel(frame);
    filePosition->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    filePosition->setLeftValue(tr("Location"));
    filePosition->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
    filePosition->rightWidget()->setMaximumHeight(kMaximumHeight);

    fileCreated = new KeyValueLabel(frame);
    fileCreated->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCreated->setLeftValue(tr("Time created"));
    fileCreated->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
    fileCreated->rightWidget()->setMaximumHeight(kMaximumHeight);

    fileAccessed = new KeyValueLabel(frame);
    fileAccessed->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileAccessed->setLeftValue(tr("Time accessed"));
    fileAccessed->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
    fileAccessed->rightWidget()->setMaximumHeight(kMaximumHeight);

    fileModified = new KeyValueLabel(frame);
    fileModified->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileModified->setLeftValue(tr("Time locked"));
    fileModified->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
    fileModified->rightWidget()->setMaximumHeight(kMaximumHeight);

    QGridLayout *gl = new QGridLayout;
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setColumnStretch(0, 1);
    gl->setColumnStretch(1, 1);
    gl->setColumnStretch(2, 2);
    QFrame *tempFrame = new QFrame(frame);
    tempFrame->setLayout(gl);

    QGridLayout *glayout = new QGridLayout;
    glayout->setContentsMargins(15, 15, 5, 10);
    glayout->setSpacing(16);
    glayout->addWidget(fileSize, 0, 0, 1, 6);
    glayout->addWidget(fileCount, 1, 0, 1, 6);
    glayout->addWidget(fileType, 2, 0, 1, 6);
    glayout->addWidget(filePosition, 3, 0, 1, 6);
    glayout->addWidget(fileCreated, 4, 0, 1, 6);
    glayout->addWidget(fileAccessed, 5, 0, 1, 6);
    glayout->addWidget(fileModified, 6, 0, 1, 6);
    glayout->addWidget(tempFrame, 7, 0, 1, 6);
    glayout->setColumnStretch(0, 1);

    frame->setLayout(glayout);
    setContent(frame);
}

void BasicWidget::selectFileUrl(const QUrl &url)
{
    QUrl realurl = VaultHelper::instance()->vaultToLocalUrl(url);
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull())
        return;

    Settings setting(kVaultTimeConfigFile);

    filePosition->setRightValue(UrlRoute::toString(url), Qt::ElideMiddle, Qt::AlignVCenter, true);

    if (setting.value(QString("VaultTime"), QString("CreateTime")).toString().isEmpty())
        fileCreated->setRightValue(setting.value(QString("VaultTime"), QString("InterviewTime")).toString(), Qt::ElideNone, Qt::AlignVCenter, true);
    else
        fileCreated->setRightValue(setting.value(QString("VaultTime"), QString("CreateTime")).toString(), Qt::ElideNone, Qt::AlignVCenter, true);

    fileAccessed->setRightValue(setting.value(QString("VaultTime"), QString("InterviewTime")).toString(), Qt::ElideNone, Qt::AlignVCenter, true);

    if (setting.value(QString("VaultTime"), QString("LockTime")).toString().isEmpty())
        fileModified->setRightValue(setting.value(QString("VaultTime"), QString("InterviewTime")).toString(), Qt::ElideNone, Qt::AlignVCenter, true);
    else
        fileModified->setRightValue(setting.value(QString("VaultTime"), QString("LockTime")).toString(), Qt::ElideNone, Qt::AlignVCenter, true);

    fileCount->setVisible(false);

    if (info->isAttributes(FileInfo::FileIsType::kIsDir)) {
        fileType->setRightValue(info->displayOf(DisPlayInfoType::kMimeTypeDisplayName), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fileCount->setVisible(true);
        fileCount->setRightValue(QString::number(0), Qt::ElideNone, Qt::AlignVCenter, true);
        fileCalculationUtils->start(QList<QUrl>() << realurl);
    }
}

qint64 BasicWidget::getFileSize()
{
    return fSize;
}

int BasicWidget::getFileCount()
{
    return fCount;
}

void BasicWidget::slotFileCountAndSizeChange(qint64 size, int filesCount, int directoryCount)
{
    fSize = size;
    fileSize->setRightValue(FileUtils::formatSize(size));

    fCount = filesCount + (directoryCount > 1 ? directoryCount - 1 : 0);
    fileCount->setRightValue(QString::number(fCount));
}

void BasicWidget::closeEvent(QCloseEvent *event)
{
    DArrowLineDrawer::closeEvent(event);
}
