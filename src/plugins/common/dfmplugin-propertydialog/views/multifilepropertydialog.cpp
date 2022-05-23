/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "multifilepropertydialog.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"

#include <DFontSizeManager>

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

DFMBASE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

MultiFilePropertyDialog::MultiFilePropertyDialog(const QList<QUrl> &urls, QWidget *const parent)
    : DDialog(parent), urlList(urls)
{
    initHeadUi();
    setFixedSize(300, 360);
    fileCalculationUtils = new FileStatisticsJob;
    connect(fileCalculationUtils, &FileStatisticsJob::sizeChanged, this, &MultiFilePropertyDialog::updateFolderSizeLabel);
    fileCalculationUtils->start(urlList);
    calculateFileCount();
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
    multiFileLable->setText(tr("Multiple Files"));

    basicInfoLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(basicInfoLabel, DFontSizeManager::T6, QFont::Medium);
    basicInfoLabel->setText(tr("Basic info"));
    totalSizeLabel = new KeyValueLabel(this);
    totalSizeLabel->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    totalSizeLabel->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    totalSizeLabel->setLeftValue(tr("Total size"), Qt::ElideNone, Qt::AlignLeft);
    fileCountLabel = new KeyValueLabel(this);
    fileCountLabel->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCountLabel->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCountLabel->setLeftValue(tr("Number of files"), Qt::ElideNone, Qt::AlignLeft);
    QPushButton *btn = new QPushButton(this);
    btn->setMaximumHeight(1);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(basicInfoLabel, 0, Qt::AlignLeft);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(15, 15, 5, 10);
    gridLayout->setSpacing(16);
    gridLayout->addWidget(totalSizeLabel, 0, 0, 1, 6);
    gridLayout->addWidget(fileCountLabel, 1, 0, 1, 6);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 0, 10, 10);
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

void MultiFilePropertyDialog::calculateFileCount()
{
    int dirCount = 0;
    int fileCount = 0;

    for (QUrl &url : urlList) {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
        if (info.isNull())
            return;
        if (info->isDir())
            ++dirCount;
        if (info->isFile())
            ++fileCount;
    }

    fileCountLabel->setRightValue(tr("%1 file(s), %2 folder(s)").arg(fileCount).arg(dirCount), Qt::ElideNone, Qt::AlignLeft, false);
}

void MultiFilePropertyDialog::updateFolderSizeLabel(qint64 size)
{
    totalSizeLabel->setRightValue(FileUtils::formatSize(size), Qt::ElideNone, Qt::AlignLeft, false);
}
