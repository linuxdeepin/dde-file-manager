// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multifilepropertydialog.h"
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <DFontSizeManager>

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

MultiFilePropertyDialog::MultiFilePropertyDialog(const QList<QUrl> &urls, QWidget *const parent)
    : DDialog(parent), urlList(urls)
{
    initHeadUi();
    setFixedSize(300, 360);
    fileCalculationUtils = new FileStatisticsJob;
    fileCalculationUtils->setFileHints(FileStatisticsJob::FileHint::kNoFollowSymlink);
    connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &MultiFilePropertyDialog::updateFolderSizeLabel);
    QList<QUrl> targets;
    UniversalUtils::urlsTransformToLocal(urlList, &targets);
    fileCalculationUtils->start(targets);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

MultiFilePropertyDialog::~MultiFilePropertyDialog()
{
    fileCalculationUtils->stop();
    fileCalculationUtils->deleteLater();
}

void MultiFilePropertyDialog::initHeadUi()
{
    iconLabel = new QLabel(this);
    QIcon icon;
    icon.addFile(QString { ":/images/images/multiple_files.png" });
    icon.addFile(QString { ":/images/images/multiple_files@2x.png" });
    iconLabel->setPixmap(icon.pixmap(128, 128));

    multiFileLable = new QLabel(this);
    DFontSizeManager::instance()->bind(multiFileLable, DFontSizeManager::T9, QFont::Medium);
    multiFileLable->setText(tr("Multiple Files"));

    basicInfoLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(basicInfoLabel, DFontSizeManager::T6, QFont::Medium);
    basicInfoLabel->setText(tr("Basic info"));

    totalSizeLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(totalSizeLabel, DFontSizeManager::T9, QFont::Medium);
    totalSizeLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    totalSizeLabel->setText(tr("Total size"));
    totalSizeValueLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(totalSizeValueLabel, DFontSizeManager::T9, QFont::Medium);

    fileCountLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(fileCountLabel, DFontSizeManager::T9, QFont::Medium);
    fileCountLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    fileCountLabel->setText(tr("Number of files"));
    fileCountValueLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(fileCountValueLabel, DFontSizeManager::T9, QFont::Medium);

    accessTimeLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(accessTimeLabel, DFontSizeManager::T9, QFont::Medium);
    accessTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    accessTimeLabel->setText(tr("Time accessed"));
    accessTimeValueLabel = new QLabel("-", this);

    modifyTimeLable = new QLabel(this);
    DFontSizeManager::instance()->bind(modifyTimeLable, DFontSizeManager::T9, QFont::Medium);
    modifyTimeLable->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    modifyTimeLable->setText(tr("Time modified"));
    modifyTimeValueLable = new QLabel("-", this);

    QPushButton *btn = new QPushButton(this);
    btn->setMaximumHeight(1);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(basicInfoLabel, 0, Qt::AlignLeft);

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(14);
    gridLayout->addWidget(totalSizeLabel, 0, 0);
    gridLayout->addWidget(totalSizeValueLabel, 0, 1);
    gridLayout->addWidget(fileCountLabel, 1, 0);
    gridLayout->addWidget(fileCountValueLabel, 1, 1);
    gridLayout->addWidget(accessTimeLabel, 2, 0);
    gridLayout->addWidget(accessTimeValueLabel, 2, 1);
    gridLayout->addWidget(modifyTimeLable, 3, 0);
    gridLayout->addWidget(modifyTimeValueLable, 3, 1);

    gridLayout->setColumnStretch(0, 4);
    gridLayout->setColumnStretch(1, 6);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 0, 0, 10);
    vlayout->addWidget(iconLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout->addWidget(multiFileLable, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout->setSpacing(10);
    vlayout->addWidget(btn);
    vlayout->addLayout(hlayout);
    vlayout->addLayout(gridLayout);

    QFrame *frame = new QFrame(this);
    frame->setLayout(vlayout);
    addContent(frame);
}

void MultiFilePropertyDialog::updateFolderSizeLabel(qint64 size, int filesCount, int directoryCount)
{
    fileCountValueLabel->setText(tr("%1 file(s), %2 folder(s)").arg(filesCount).arg(directoryCount));
    totalSizeValueLabel->setText(FileUtils::formatSize(size));
}
