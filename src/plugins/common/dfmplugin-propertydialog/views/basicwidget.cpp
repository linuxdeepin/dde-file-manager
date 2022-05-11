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
#include "utils/propertydialoghelper.h"
#include "events/propertyeventcall.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"

#include "services/common/propertydialog/property_defines.h"
#include "services/common/delegate/delegateservice.h"

#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
#include <QSet>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
CPY_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

BasicWidget::BasicWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    initUI();
    fileCalculationUtils = new FileStatisticsJob;
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

    frameMain = new QFrame(this);

    fileSize = new KeyValueLabel(frameMain);
    fileSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSize->setLeftValue(tr("Size"));
    fileSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileCount = new KeyValueLabel(frameMain);
    fileCount->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCount->setLeftValue(tr("Contains"));
    fileCount->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileType = new KeyValueLabel(frameMain);
    fileType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileType->setLeftValue(tr("Type"));
    fileType->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    filePosition = new KeyValueLabel(frameMain);
    filePosition->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    filePosition->setLeftValue(tr("Location"));
    filePosition->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileCreated = new KeyValueLabel(frameMain);
    fileCreated->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCreated->setLeftValue(tr("Time created"));
    fileCreated->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileAccessed = new KeyValueLabel(frameMain);
    fileAccessed->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileAccessed->setLeftValue(tr("Time accessed"));
    fileAccessed->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileModified = new KeyValueLabel(frameMain);
    fileModified->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileModified->setLeftValue(tr("Time modified"));
    fileModified->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    hideFile = new QCheckBox(frameMain);
    hideFile->setText(tr("Hide this file"));
}

void BasicWidget::basicExpand(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpand> fieldCondition = PropertyDialogHelper::propertyServiceInstance()->basicExpandField(url);

    QList<BasicExpandType> keys = fieldCondition.keys();
    for (BasicExpandType key : keys) {
        BasicExpand expand = fieldCondition.value(key);
        QList<BasicFieldExpandEnum> filterEnumList = expand.keys();
        switch (key) {
        case kFieldInsert: {
            for (BasicFieldExpandEnum k : filterEnumList) {
                QList<QPair<QString, QString>> fieldlist = expand.values(k);
                for (QPair<QString, QString> field : fieldlist) {
                    KeyValueLabel *expandLabel = new KeyValueLabel(this);
                    expandLabel->setLeftValue(field.first);
                    expandLabel->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
                    expandLabel->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignVCenter, true);
                    expandLabel->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
                    fieldMap.insert(k, expandLabel);
                }
            }
        } break;
        case kFieldReplace: {
            for (BasicFieldExpandEnum k : filterEnumList) {
                QPair<QString, QString> field = expand.value(k);
                fieldMap.value(k)->setLeftValue(field.first);
                fieldMap.value(k)->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignVCenter, true);
            }
        } break;
        }
    }

    QLabel *label = new QLabel(frameMain);
    label->setFixedSize(hideFile->size());
    QGridLayout *gl = new QGridLayout;
    gl->setMargin(0);
    gl->addWidget(label, 0, 0, 1, 2);
    gl->addWidget(hideFile, 0, 1, 1, 5);
    gl->setColumnStretch(0, 2);
    gl->setColumnStretch(1, 3);
    QFrame *tempFrame = new QFrame(frameMain);
    tempFrame->setLayout(gl);

    QGridLayout *glayout = new QGridLayout;
    glayout->setContentsMargins(15, 15, 5, 10);
    glayout->setSpacing(16);
    int row = 0;
    QList<BasicFieldExpandEnum> fields = fieldMap.keys();
    QSet<BasicFieldExpandEnum> fieldset = QSet<BasicFieldExpandEnum>::fromList(fields);
    fields = fieldset.toList();
    qSort(fields.begin(), fields.end());
    for (BasicFieldExpandEnum &key : fields) {
        QList<KeyValueLabel *> kvls = fieldMap.values(key);
        for (int i = kvls.count() - 1; i >= 0; --i) {
            KeyValueLabel *kvl = kvls[i];
            glayout->addWidget(kvl, row, 0, 1, 6);
            row++;
        }
    }
    glayout->addWidget(tempFrame, row, 0, 1, 6);
    glayout->setColumnStretch(0, 1);

    frameMain->setLayout(glayout);
    setContent(frameMain);
}

void BasicWidget::basicFieldFilter(const QUrl &url)
{
    FilePropertyControlFilter fieldFilter = propertyServIns->contorlFieldFilter(url);
    if (fieldFilter & FilePropertyControlFilter::kFileSizeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileSize);
        fileSize->deleteLater();
        fileSize = nullptr;
    } else if (fieldFilter & FilePropertyControlFilter::kFileTypeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileType);
        fileType->deleteLater();
        fileType = nullptr;
    } else if (fieldFilter & FilePropertyControlFilter::kFileCountFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileCount);
        fileCount->deleteLater();
        fileCount = nullptr;
    } else if (fieldFilter & FilePropertyControlFilter::kFilePositionFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFilePosition);
        filePosition->deleteLater();
        filePosition = nullptr;
    } else if (fieldFilter & FilePropertyControlFilter::kFileCreateTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileCreateTime);
        fileCreated->deleteLater();
        fileCreated = nullptr;
    } else if (fieldFilter & FilePropertyControlFilter::kFileAccessedTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileAccessedTime);
        fileAccessed->deleteLater();
        fileAccessed = nullptr;
    } else if (fieldFilter & FilePropertyControlFilter::kFileModifiedTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileModifiedTime);
        fileModified->deleteLater();
        fileModified = nullptr;
    }
}

void BasicWidget::basicFill(const QUrl &url)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;

    if (info->isHidden())
        hideFile->setChecked(true);

    connect(hideFile, &QCheckBox::stateChanged, this, &BasicWidget::slotFileHide);

    if (filePosition && filePosition->RightValue().isEmpty())
        filePosition->setRightValue(url.path(), Qt::ElideMiddle, Qt::AlignVCenter, true);
    if (fileCreated && fileCreated->RightValue().isEmpty())
        fileCreated->setRightValue(info->birthTime().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideNone, Qt::AlignVCenter, true);
    if (fileAccessed && fileAccessed->RightValue().isEmpty())
        fileAccessed->setRightValue(info->lastRead().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideNone, Qt::AlignVCenter, true);
    if (fileModified && fileModified->RightValue().isEmpty())
        fileModified->setRightValue(info->lastModified().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideNone, Qt::AlignVCenter, true);
    if (fileSize && fileSize->RightValue().isEmpty())
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignVCenter, true);
    if (fileCount && fileCount->RightValue().isEmpty())
        fileCount->setVisible(false);

    if (fileType && fileType->RightValue().isEmpty()) {
        QUrl localUrl = delegateServIns->urlTransform(url);
        QMimeType mimeType = MimeDatabase::mimeTypeForUrl(localUrl);
        MimeDatabase::FileType type = MimeDatabase::mimeFileTypeNameToEnum(mimeType.name());
        switch (static_cast<int>(type)) {
        case MimeDatabase::FileType::kDirectory: {
            fileType->setRightValue(tr("Directory") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
            if (fileCount && fileCount->RightValue().isEmpty()) {
                fileCount->setVisible(true);
                fileCount->setRightValue(QString::number(0), Qt::ElideNone, Qt::AlignVCenter, true);
                fileCalculationUtils->start(QList<QUrl>() << localUrl);
                connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &BasicWidget::slotFileCountAndSizeChange);
            }
        } break;
        case MimeDatabase::FileType::kDocuments: {
            fileType->setRightValue(tr("Documents") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kVideos: {
            fileType->setRightValue(tr("Videos") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kImages: {
            fileType->setRightValue(tr("Images") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kAudios: {
            fileType->setRightValue(tr("Audios") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kExecutable: {
            fileType->setRightValue(tr("Executable") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kArchives: {
            fileType->setRightValue(tr("Archives") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kDesktopApplication: {
            fileType->setRightValue(tr("application") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kUnknown: {
            fileType->setRightValue(tr("Unknown") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        }
    }
}

void BasicWidget::initFileMap()
{
    fieldMap.insert(BasicFieldExpandEnum::kFileSize, fileSize);
    fieldMap.insert(BasicFieldExpandEnum::kFileCount, fileCount);
    fieldMap.insert(BasicFieldExpandEnum::kFileType, fileType);
    fieldMap.insert(BasicFieldExpandEnum::kFilePosition, filePosition);
    fieldMap.insert(BasicFieldExpandEnum::kFileCreateTime, fileCreated);
    fieldMap.insert(BasicFieldExpandEnum::kFileAccessedTime, fileAccessed);
    fieldMap.insert(BasicFieldExpandEnum::kFileModifiedTime, fileModified);
}

void BasicWidget::selectFileUrl(const QUrl &url)
{
    currentUrl = url;

    initFileMap();

    basicFieldFilter(url);

    basicExpand(url);

    basicFill(url);
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

void BasicWidget::slotFileHide(int state)
{
    quint64 winIDs = QApplication::activeWindow()->winId();
    PropertyEventCall::sendFileHide(winIDs, { currentUrl });
}

void BasicWidget::closeEvent(QCloseEvent *event)
{
    DArrowLineDrawer::closeEvent(event);
}
