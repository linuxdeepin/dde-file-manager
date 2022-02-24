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
#include "basicwidget.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"

#include <QFileInfo>
#include <QDateTime>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE
BasicWidget::BasicWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    initUI();
    fileCalculationUtils = new FileCalculationUtils;
}

BasicWidget::~BasicWidget()
{
    fileCalculationUtils->deleteLater();
}

void BasicWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    setTitle(QString(tr("BasicInfo")));

    setExpand(true);

    QFrame *frame = new QFrame(this);

    fileSize = new KeyValueLabel(frame);
    fileSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSize->setLeftValue(tr("Size"));
    fileSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileCount = new KeyValueLabel(frame);
    fileCount->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCount->setLeftValue(tr("Contains"));
    fileCount->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileType = new KeyValueLabel(frame);
    fileType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileType->setLeftValue(tr("Type"));
    fileType->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    filePosition = new KeyValueLabel(frame);
    filePosition->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    filePosition->setLeftValue(tr("Location"));
    filePosition->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileCreated = new KeyValueLabel(frame);
    fileCreated->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCreated->setLeftValue(tr("Time created"));
    fileCreated->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileAccessed = new KeyValueLabel(frame);
    fileAccessed->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileAccessed->setLeftValue(tr("Time accessed"));
    fileAccessed->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileModified = new KeyValueLabel(frame);
    fileModified->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileModified->setLeftValue(tr("Time modified"));
    fileModified->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    hideFile = new QCheckBox(frame);
    hideFile->setText(tr("Hide this file"));

    QLabel *label = new QLabel(frame);
    label->setFixedSize(hideFile->size());
    QGridLayout *gl = new QGridLayout;
    gl->setMargin(0);
    gl->addWidget(label, 0, 0, 1, 2);
    gl->addWidget(hideFile, 0, 2, 1, 4);
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
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;

    filePosition->setRightValue(info->absoluteFilePath(), Qt::ElideMiddle, Qt::AlignVCenter, true);
    fileCreated->setRightValue(info->birthTime().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideNone, Qt::AlignVCenter, true);
    fileAccessed->setRightValue(info->lastRead().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideNone, Qt::AlignVCenter, true);
    fileModified->setRightValue(info->lastModified().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideNone, Qt::AlignVCenter, true);
    fileCount->setVisible(false);

    QMimeType mimeType = MimeDatabase::mimeTypeForUrl(url);
    MimeDatabase::FileType type = MimeDatabase::mimeFileTypeNameToEnum(mimeType.name());
    switch (static_cast<int>(type)) {
    case MimeDatabase::FileType::kDirectory:
        fileType->setRightValue(tr("Directory") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fileCount->setVisible(true);
        fileCount->setRightValue(QString::number(0), Qt::ElideNone, Qt::AlignVCenter, true);
        fileCalculationUtils->startThread(QList<QUrl>() << url);
        connect(fileCalculationUtils, &FileCalculationUtils::sigTotalChange, this, &BasicWidget::slotFileDirSizeChange);
        connect(fileCalculationUtils, &FileCalculationUtils::sigFileChange, this, &BasicWidget::slotFileCountChange);
        break;
    case MimeDatabase::FileType::kDocuments: {
        fileType->setRightValue(tr("Documents") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fSize += info->size();
        fCount += 1;
    } break;
    case MimeDatabase::FileType::kVideos: {
        fileType->setRightValue(tr("Videos") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fSize += info->size();
        fCount += 1;
    } break;
    case MimeDatabase::FileType::kImages: {
        fileType->setRightValue(tr("Images") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fSize += info->size();
        fCount += 1;
    } break;
    case MimeDatabase::FileType::kAudios: {
        fileType->setRightValue(tr("Audios") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fSize += info->size();
        fCount += 1;
    } break;
    case MimeDatabase::FileType::kExecutable:
        fileType->setRightValue(tr("Executable") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fSize += info->size();
        fCount += 1;
        break;
    case MimeDatabase::FileType::kArchives:
        fileType->setRightValue(tr("Archives") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fSize += info->size();
        fCount += 1;
        break;
    case MimeDatabase::FileType::kUnknown:
        fileType->setRightValue(tr("Unknown") + mimeType.name(), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
        fileSize->setVisible(true);
        fSize += info->size();
        fCount += 1;
        break;
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

void BasicWidget::slotFileDirSizeChange(qint64 size)
{
    fSize += size;
    fileSize->setRightValue(FileUtils::formatSize(size));
}

void BasicWidget::slotFileCountChange(qint64 size)
{
    fCount += size;
    fileCount->setRightValue(QString::number(size));
}

void BasicWidget::closeEvent(QCloseEvent *event)
{
    DArrowLineDrawer::closeEvent(event);
}
