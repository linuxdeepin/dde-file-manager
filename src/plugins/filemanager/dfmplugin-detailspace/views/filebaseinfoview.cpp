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
#include "filebaseinfoview.h"
#include "services/filemanager/detailspace/detailspaceservice.h"
#include "services/common/delegate/delegateservice.h"

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/event/event.h>

USING_IO_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_detailspace;

const int kImageExten = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kVideoExten = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kAudioExten = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
FileBaseInfoView::FileBaseInfoView(QWidget *parent)
    : QFrame(parent)
{
}

FileBaseInfoView::~FileBaseInfoView()
{
    dpfSignalDispatcher->unsubscribe(kImageExten, this, &FileBaseInfoView::imageExtenInfoReceiver);
    dpfSignalDispatcher->unsubscribe(kVideoExten, this, &FileBaseInfoView::videoExtenInfoReceiver);
    dpfSignalDispatcher->unsubscribe(kAudioExten, this, &FileBaseInfoView::audioExtenInfoReceiver);
}

void FileBaseInfoView::initUI()
{
    fileName = new KeyValueLabel(this);
    fileName->setLeftValue(tr("Name"), Qt::ElideNone, Qt::AlignLeft);
    fileName->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileName->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSize = new KeyValueLabel(this);
    fileSize->setLeftValue(tr("Size"), Qt::ElideNone, Qt::AlignLeft);
    fileSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileViewSize = new KeyValueLabel(this);
    fileViewSize->setLeftValue(tr("Dimension"), Qt::ElideNone, Qt::AlignLeft);
    fileViewSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileViewSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileDuration = new KeyValueLabel(this);
    fileDuration->setLeftValue(tr("Duration"), Qt::ElideNone, Qt::AlignLeft);
    fileDuration->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileDuration->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileType = new KeyValueLabel(this);
    fileType->setLeftValue(tr("Type"), Qt::ElideNone, Qt::AlignLeft);
    fileType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileType->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileInterviewTime = new KeyValueLabel(this);
    fileInterviewTime->setLeftValue(tr("Accessed"), Qt::ElideNone, Qt::AlignLeft);
    fileInterviewTime->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileInterviewTime->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileChangeTime = new KeyValueLabel(this);
    fileChangeTime->setLeftValue(tr("Modified"), Qt::ElideNone, Qt::AlignLeft);
    fileChangeTime->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileChangeTime->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
}

void FileBaseInfoView::initFileMap()
{
    fieldMap.insert(BasicFieldExpandEnum::kFileName, fileName);
    fieldMap.insert(BasicFieldExpandEnum::kFileSize, fileSize);
    fieldMap.insert(BasicFieldExpandEnum::kFileType, fileType);
    fieldMap.insert(BasicFieldExpandEnum::kFileDuration, fileDuration);
    fieldMap.insert(BasicFieldExpandEnum::kFileViewSize, fileViewSize);
    fieldMap.insert(BasicFieldExpandEnum::kFileChangeTIme, fileChangeTime);
    fieldMap.insert(BasicFieldExpandEnum::kFileInterviewTime, fileInterviewTime);
}

void FileBaseInfoView::basicExpand(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> fieldCondition = detailServIns->createBasicExpandField(url);

    QList<BasicExpandType> keys = fieldCondition.keys();
    for (BasicExpandType key : keys) {
        BasicExpandMap expand = fieldCondition.value(key);
        QList<BasicFieldExpandEnum> filterEnumList = expand.keys();
        switch (key) {
        case kFieldInsert: {
            for (BasicFieldExpandEnum k : filterEnumList) {
                QList<QPair<QString, QString>> fieldlist = expand.values(k);
                for (QPair<QString, QString> field : fieldlist) {
                    KeyValueLabel *expandLabel = new KeyValueLabel(this);
                    expandLabel->setLeftValue(field.first);
                    expandLabel->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
                    expandLabel->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignLeft, true);
                    expandLabel->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
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

    QGridLayout *glayout = new QGridLayout;
    glayout->setSpacing(10);
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
    glayout->setColumnStretch(0, 1);
    setLayout(glayout);
}

void FileBaseInfoView::basicFieldFilter(const QUrl &url)
{
    QUrl filterUrl = url;
    if (delegateServIns->isRegisterUrlTransform(filterUrl.scheme()))
        filterUrl = delegateServIns->urlTransform(filterUrl);

    DetailFilterTypes fieldFilter = detailServIns->contorlFieldFilter(filterUrl);
    if (fieldFilter & DetailFilterType::kFileNameField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileName);
        fileName->deleteLater();
        fileName = nullptr;
    }

    if (fieldFilter & DetailFilterType::kFileSizeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileSize);
        fileSize->deleteLater();
        fileSize = nullptr;
    }

    if (fieldFilter & DetailFilterType::kFileTypeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileType);
        fileType->deleteLater();
        fileType = nullptr;
    }

    if (fieldFilter & DetailFilterType::kFileDurationField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileDuration);
        fileDuration->deleteLater();
        fileDuration = nullptr;
    }

    if (fieldFilter & DetailFilterType::kFileViewSizeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileViewSize);
        fileViewSize->deleteLater();
        fileViewSize = nullptr;
    }

    if (fieldFilter & DetailFilterType::kFileChangeTImeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileChangeTIme);
        fileChangeTime->deleteLater();
        fileChangeTime = nullptr;
    }

    if (fieldFilter & DetailFilterType::kFileInterviewTimeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileInterviewTime);
        fileInterviewTime->deleteLater();
        fileInterviewTime = nullptr;
    }
}

void FileBaseInfoView::basicFill(const QUrl &url)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;

    if (fileName && fileName->RightValue().isEmpty())
        fileName->setRightValue(info->fileName(), Qt::ElideMiddle, Qt::AlignLeft, true);

    if (fileInterviewTime && fileInterviewTime->RightValue().isEmpty())
        fileInterviewTime->setRightValue(info->lastRead().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideMiddle, Qt::AlignLeft, true, 150);

    if (fileChangeTime && fileChangeTime->RightValue().isEmpty())
        fileChangeTime->setRightValue(info->lastModified().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideMiddle, Qt::AlignLeft, true, 150);

    if (fileSize && fileSize->RightValue().isEmpty()) {
        fileSize->setVisible(true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
    }

    if (fileViewSize && fileViewSize->RightValue().isEmpty())
        fileViewSize->setVisible(false);

    if (fileDuration && fileDuration->RightValue().isEmpty())
        fileDuration->setVisible(false);

    QUrl localUrl = delegateServIns->urlTransform(url);

    AbstractFileInfoPointer localinfo = InfoFactory::create<AbstractFileInfo>(localUrl);

    if (fileType && fileType->RightValue().isEmpty() && localinfo) {
        MimeDatabase::FileType type = MimeDatabase::mimeFileTypeNameToEnum(localinfo->mimeTypeName());
        switch (type) {
        case MimeDatabase::FileType::kDirectory:
            fileType->setRightValue(tr("Directory"), Qt::ElideNone, Qt::AlignLeft, true);
            break;
        case MimeDatabase::FileType::kDocuments: {
            fileType->setRightValue(tr("Documents"), Qt::ElideNone, Qt::AlignLeft, true);
        } break;
        case MimeDatabase::FileType::kVideos: {
            fileType->setRightValue(tr("Videos"), Qt::ElideNone, Qt::AlignLeft, true);
            QList<DFileInfo::AttributeExtendID> extenList;
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            localinfo->mediaInfoAttributes(DFileInfo::MediaType::kVideo, extenList, FileBaseInfoView::videoExtenInfo);
        } break;
        case MimeDatabase::FileType::kImages: {
            fileType->setRightValue(tr("Images"), Qt::ElideNone, Qt::AlignLeft, true);
            QList<DFileInfo::AttributeExtendID> extenList;
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight;
            localinfo->mediaInfoAttributes(DFileInfo::MediaType::kImage, extenList, FileBaseInfoView::imageExtenInfo);
        } break;
        case MimeDatabase::FileType::kAudios: {
            fileType->setRightValue(tr("Audios"), Qt::ElideNone, Qt::AlignLeft, true);
            QList<DFileInfo::AttributeExtendID> extenList;
            extenList << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            localinfo->mediaInfoAttributes(DFileInfo::MediaType::kAudio, extenList, FileBaseInfoView::audioExtenInfo);
        } break;
        case MimeDatabase::FileType::kExecutable:
            fileType->setRightValue(tr("Executable"), Qt::ElideNone, Qt::AlignLeft, true);
            break;
        case MimeDatabase::FileType::kArchives:
            fileType->setRightValue(tr("Archives"), Qt::ElideNone, Qt::AlignLeft, true);
            break;
        case MimeDatabase::FileType::kUnknown:
            fileType->setRightValue(tr("Unknown"), Qt::ElideNone, Qt::AlignLeft, true);
            break;
        default:
            break;
        }
    }
}

void FileBaseInfoView::clearField()
{
    QList<BasicFieldExpandEnum> expandEnum = fieldMap.keys();
    for (BasicFieldExpandEnum key : expandEnum) {
        KeyValueLabel *ky = fieldMap.take(key);
        ky->deleteLater();
    }
}

void FileBaseInfoView::connectEvent()
{
    dpfSignalDispatcher->subscribe(kImageExten, this, &FileBaseInfoView::imageExtenInfoReceiver);
    dpfSignalDispatcher->subscribe(kVideoExten, this, &FileBaseInfoView::videoExtenInfoReceiver);
    dpfSignalDispatcher->subscribe(kAudioExten, this, &FileBaseInfoView::audioExtenInfoReceiver);
}

void FileBaseInfoView::connectInit()
{
    connect(this, &FileBaseInfoView::sigImageExtenInfo, this, &FileBaseInfoView::slotImageExtenInfo, Qt::QueuedConnection);
    connect(this, &FileBaseInfoView::sigVideoExtenInfo, this, &FileBaseInfoView::slotVideoExtenInfo, Qt::QueuedConnection);
    connect(this, &FileBaseInfoView::sigAudioExtenInfo, this, &FileBaseInfoView::slotAudioExtenInfo, Qt::QueuedConnection);
}

void FileBaseInfoView::imageExtenInfoReceiver(const QStringList &properties)
{
    const QStringList pr = properties;
    emit sigImageExtenInfo(pr);
}

void FileBaseInfoView::videoExtenInfoReceiver(const QStringList &properties)
{
    const QStringList pr = properties;
    emit sigVideoExtenInfo(pr);
}

void FileBaseInfoView::audioExtenInfoReceiver(const QStringList &properties)
{
    const QStringList pr = properties;
    emit sigAudioExtenInfo(pr);
}

void FileBaseInfoView::slotImageExtenInfo(const QStringList &properties)
{
    if (fileViewSize && fileViewSize->RightValue().isEmpty()) {
        fileViewSize->setVisible(true);
        fileViewSize->setRightValue(properties.isEmpty() ? " " : properties.first(), Qt::ElideNone, Qt::AlignLeft, true);
    }
}

void FileBaseInfoView::slotVideoExtenInfo(const QStringList &properties)
{
    if (fileViewSize && fileViewSize->RightValue().isEmpty()) {
        fileViewSize->setVisible(true);
        fileViewSize->setRightValue(properties.isEmpty() ? " " : properties.first(), Qt::ElideNone, Qt::AlignLeft, true);
    }

    if (fileDuration && fileDuration->RightValue().isEmpty()) {
        fileDuration->setVisible(true);
        fileDuration->setRightValue(properties.count() > 1 ? properties[1] : " ", Qt::ElideNone, Qt::AlignLeft, true);
    }
}

void FileBaseInfoView::slotAudioExtenInfo(const QStringList &properties)
{
    if (fileDuration && fileDuration->RightValue().isEmpty()) {
        fileDuration->setVisible(true);
        fileDuration->setRightValue(properties.isEmpty() ? " " : properties.first(), Qt::ElideNone, Qt::AlignLeft, true);
    }
}

void FileBaseInfoView::imageExtenInfo(bool flg, QMap<DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (flg) {
        QStringList property;
        if (!properties.isEmpty()) {
            int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
            int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();
            QString viewSize = QString::number(width) + "x" + QString::number(height);
            property << viewSize;
            dpfSignalDispatcher->publish(kImageExten, property);
        }
    }
}

void FileBaseInfoView::videoExtenInfo(bool flg, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (flg) {
        QStringList property;
        if (!properties.isEmpty()) {
            int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
            int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();
            QString viewSize = QString::number(width) + "x" + QString::number(height);
            property << viewSize;

            QVariant duration = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration];

            int s = duration.toInt();

            QTime t(0, 0, 0);
            t = t.addMSecs(s);

            QString durationStr = t.toString("hh:mm:ss");

            property << durationStr;

            dpfSignalDispatcher->publish(kVideoExten, property);
        }
    }
}

void FileBaseInfoView::audioExtenInfo(bool flg, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (flg) {
        QStringList property;
        if (!properties.isEmpty()) {
            QVariant duration = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration];

            int s = duration.toInt();

            QTime t(0, 0, 0);
            t = t.addMSecs(s);

            QString durationStr = t.toString("hh:mm:ss");

            property << durationStr;

            dpfSignalDispatcher->publish(kAudioExten, property);
        }
    }
}

/*!
 * \brief           通过文件url获取文件fileinfo对象，在通过MimDatabase获取文件类型。最后根据文件类型对不同文件设置信息
 * \param[in] url       文件url
 */
void FileBaseInfoView::setFileUrl(const QUrl &url)
{
    currentUrl = url;

    clearField();

    initUI();

    connectEvent();

    connectInit();

    initFileMap();

    basicFieldFilter(url);

    basicExpand(url);

    basicFill(url);
}
