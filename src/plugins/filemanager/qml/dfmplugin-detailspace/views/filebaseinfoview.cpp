// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filebaseinfoview.h"
#include "utils/detailmanager.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <dfm-framework/event/event.h>

Q_DECLARE_METATYPE(QList<QUrl> *)

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE

using namespace dfmplugin_detailspace;

FileBaseInfoView::FileBaseInfoView(QWidget *parent)
    : QFrame(parent)
{
}

FileBaseInfoView::~FileBaseInfoView()
{
}

void FileBaseInfoView::initUI()
{
    fileName = new KeyValueLabel(this);
    fileName->setLeftValue(tr("Name"), Qt::ElideMiddle, Qt::AlignLeft);
    fileName->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileName->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSize = new KeyValueLabel(this);
    fileSize->setLeftValue(tr("Size"), Qt::ElideMiddle, Qt::AlignLeft);
    fileSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileViewSize = new KeyValueLabel(this);
    fileViewSize->setMaximumHeight(30);
    fileViewSize->setLeftValue(tr("Dimension"), Qt::ElideMiddle, Qt::AlignLeft);
    fileViewSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileViewSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileDuration = new KeyValueLabel(this);
    fileDuration->setLeftValue(tr("Duration"), Qt::ElideMiddle, Qt::AlignLeft);
    fileDuration->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileDuration->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileType = new KeyValueLabel(this);
    fileType->setLeftValue(tr("Type"), Qt::ElideMiddle, Qt::AlignLeft);
    fileType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileType->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileInterviewTime = new KeyValueLabel(this);
    fileInterviewTime->setLeftValue(tr("Accessed"), Qt::ElideMiddle, Qt::AlignLeft);
    fileInterviewTime->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
    fileInterviewTime->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileChangeTime = new KeyValueLabel(this);
    fileChangeTime->setLeftValue(tr("Modified"), Qt::ElideMiddle, Qt::AlignLeft);
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
    QMap<BasicExpandType, BasicExpandMap> fieldCondition = DetailManager::instance().createBasicViewExtensionField(url);

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
    glayout->setSpacing(0);
    int row = 0;
    QList<BasicFieldExpandEnum> fields = fieldMap.keys();
    QSet<BasicFieldExpandEnum> fieldset {fields.begin(), fields.end()};
    fields = {fieldset.begin(), fieldset.end()};
    std::sort(fields.begin(), fields.end());
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
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ filterUrl }, &urls);

    if (ok && !urls.isEmpty())
        filterUrl = urls.first();

    DetailFilterType fieldFilters = DetailFilterType::kNotFilter;
    fieldFilters = DetailManager::instance().basicFiledFiltes(filterUrl);

    if (fieldFilters & DetailFilterType::kFileNameField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileName);
        fileName->deleteLater();
        fileName = nullptr;
    }

    if (fieldFilters & DetailFilterType::kFileSizeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileSize);
        fileSize->deleteLater();
        fileSize = nullptr;
    }

    if (fieldFilters & DetailFilterType::kFileTypeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileType);
        fileType->deleteLater();
        fileType = nullptr;
    }

    if (fieldFilters & DetailFilterType::kFileDurationField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileDuration);
        fileDuration->deleteLater();
        fileDuration = nullptr;
    }

    if (fieldFilters & DetailFilterType::kFileViewSizeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileViewSize);
        fileViewSize->deleteLater();
        fileViewSize = nullptr;
    }

    if (fieldFilters & DetailFilterType::kFileChangeTimeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileChangeTIme);
        fileChangeTime->deleteLater();
        fileChangeTime = nullptr;
    }

    if (fieldFilters & DetailFilterType::kFileInterviewTimeField) {
        fieldMap.remove(BasicFieldExpandEnum::kFileInterviewTime);
        fileInterviewTime->deleteLater();
        fileInterviewTime = nullptr;
    }
}

void FileBaseInfoView::basicFill(const QUrl &url)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull())
        return;

    if (fileName && fileName->RightValue().isEmpty())
        fileName->setRightValue(info->displayOf(DisPlayInfoType::kFileDisplayName), Qt::ElideMiddle, Qt::AlignLeft, true);

    if (fileInterviewTime && fileInterviewTime->RightValue().isEmpty()) {
        auto lastRead = info->timeOf(TimeInfoType::kLastRead).value<QDateTime>();
        lastRead.isValid() ? fileInterviewTime->setRightValue(lastRead.toString(FileUtils::dateTimeFormat()), Qt::ElideMiddle, Qt::AlignLeft, true, 150)
                           : fileInterviewTime->setVisible(false);
    }
    if (fileChangeTime && fileChangeTime->RightValue().isEmpty()) {
        auto lastModified = info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
        lastModified.isValid() ? fileChangeTime->setRightValue(lastModified.toString(FileUtils::dateTimeFormat()), Qt::ElideMiddle, Qt::AlignLeft, true, 150)
                               : fileChangeTime->setVisible(false);
    }

    if (fileSize)
        fileSize->setVisible(false);

    if (fileSize && fileSize->RightValue().isEmpty() && !info->isAttributes(OptInfoType::kIsDir)) {
        fileSize->setVisible(true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->adjustHeight();
    }
    if (fileViewSize && fileViewSize->RightValue().isEmpty())
        fileViewSize->setVisible(false);

    if (fileDuration && fileDuration->RightValue().isEmpty())
        fileDuration->setVisible(false);

    QUrl localUrl = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    FileInfoPointer localinfo = InfoFactory::create<FileInfo>(localUrl);
    if (localinfo && localinfo->isAttributes(OptInfoType::kIsSymLink)) {
        const QUrl &targetUrl = QUrl::fromLocalFile(localinfo->pathOf(PathInfoType::kSymLinkTarget));
        localinfo = InfoFactory::create<FileInfo>(targetUrl);
    }
    if (localinfo && FileUtils::isTrashFile(localUrl) && !UniversalUtils::urlEquals(localUrl, FileUtils::trashRootUrl())) {
        const QUrl &targetUrl = localinfo->urlOf(UrlInfoType::kRedirectedFileUrl);
        localinfo = InfoFactory::create<FileInfo>(targetUrl);
    }

    if (fileType && fileType->RightValue().isEmpty() && localinfo) {
        const QString &mimeName { localinfo->nameOf(NameInfoType::kMimeTypeName) };
        const FileInfo::FileType type = MimeTypeDisplayManager::instance()->displayNameToEnum(mimeName);
        fileType->setRightValue(localinfo->displayOf(DisPlayInfoType::kMimeTypeDisplayName), Qt::ElideNone, Qt::AlignLeft, true);
        QList<DFileInfo::AttributeExtendID> extenList;
        if (type == FileInfo::FileType::kVideos) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &FileBaseInfoView::videoExtenInfo);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kVideo, extenList);
            if (!mediaAttributes.isEmpty())
                videoExtenInfo(url, mediaAttributes);
        } else if (type == FileInfo::FileType::kImages) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &FileBaseInfoView::imageExtenInfo);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kImage, extenList);
            if (!mediaAttributes.isEmpty())
                imageExtenInfo(url, mediaAttributes);
        } else if (type == FileInfo::FileType::kAudios) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &FileBaseInfoView::audioExtenInfo);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kAudio, extenList);
            if (!mediaAttributes.isEmpty())
                audioExtenInfo(url, mediaAttributes);
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
        fileViewSize->adjustHeight();
    }
}

void FileBaseInfoView::slotVideoExtenInfo(const QStringList &properties)
{
    if (fileViewSize && fileViewSize->RightValue().isEmpty()) {
        fileViewSize->setVisible(true);
        fileViewSize->setRightValue(properties.isEmpty() ? " " : properties.first(), Qt::ElideNone, Qt::AlignLeft, true);
        fileViewSize->adjustHeight();
    }

    if (fileDuration && fileDuration->RightValue().isEmpty()) {
        fileDuration->setVisible(true);
        fileDuration->setRightValue(properties.count() > 1 ? properties[1] : " ", Qt::ElideNone, Qt::AlignLeft, true);
        fileDuration->adjustHeight();
    }
}

void FileBaseInfoView::slotAudioExtenInfo(const QStringList &properties)
{
    if (fileDuration && fileDuration->RightValue().isEmpty()) {
        fileDuration->setVisible(true);
        fileDuration->setRightValue(properties.isEmpty() ? " " : properties.first(), Qt::ElideNone, Qt::AlignLeft, true);
        fileDuration->adjustHeight();
    }
}

void FileBaseInfoView::imageExtenInfo(const QUrl &url, QMap<DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url == currentUrl) {
        QStringList property;
        if (!properties.isEmpty()) {
            int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
            int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();
            const QString &viewSize = QString::number(width) + "x" + QString::number(height);
            property << viewSize;
            emit sigImageExtenInfo(property);
        }
    }
}

void FileBaseInfoView::videoExtenInfo(const QUrl &url, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url == currentUrl) {
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

            const QString &durationStr = t.toString("hh:mm:ss");
            property << durationStr;

            emit sigVideoExtenInfo(property);
        }
    }
}

void FileBaseInfoView::audioExtenInfo(const QUrl &url, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url == currentUrl) {
        QStringList property;
        if (!properties.isEmpty()) {
            QVariant duration = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration];

            int s = duration.toInt();

            QTime t(0, 0, 0);
            t = t.addMSecs(s);

            const QString &durationStr = t.toString("hh:mm:ss");
            property << durationStr;

            emit sigAudioExtenInfo(property);
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

    connectInit();

    initFileMap();

    basicFieldFilter(url);

    basicExpand(url);

    basicFill(url);
    // bug-245717: 提前show，防止界面抖动
    adjustSize();
    show();
}
