// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileinfowidget.h"
#include "utils/detailmanager.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <QTime>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE

using namespace dfmplugin_detailspace;

FileInfoWidget::FileInfoWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
    initConnections();
}

void FileInfoWidget::initUI()
{
    // Create layout once - will never be deleted
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setSpacing(0);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setColumnStretch(0, 1);

    // Create all 7 core fields once - they will be reused for every URL
    auto createField = [this](const QString &label) -> KeyValueLabel * {
        auto *field = new KeyValueLabel(this);
        field->setLeftValue(label, Qt::ElideMiddle, Qt::AlignLeft);
        field->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
        field->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
        return field;
    };

    m_fileName = createField(tr("Name"));
    m_fileSize = createField(tr("Size"));
    m_fileViewSize = createField(tr("Resolution"));
    m_fileViewSize->setMaximumHeight(30);
    m_fileDuration = createField(tr("Duration"));
    m_fileType = createField(tr("Type"));
    m_fileAccessTime = createField(tr("Accessed"));
    m_fileModifyTime = createField(tr("Modified"));

    // Build field map for extensions
    m_fieldMap.insert(BasicFieldExpandEnum::kFileName, m_fileName);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileSize, m_fileSize);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileViewSize, m_fileViewSize);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileDuration, m_fileDuration);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileType, m_fileType);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileInterviewTime, m_fileAccessTime);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileChangeTIme, m_fileModifyTime);

    // Add all core fields to layout in order
    int row = 0;
    m_gridLayout->addWidget(m_fileName, row++, 0, 1, 6);
    m_gridLayout->addWidget(m_fileSize, row++, 0, 1, 6);
    m_gridLayout->addWidget(m_fileViewSize, row++, 0, 1, 6);
    m_gridLayout->addWidget(m_fileDuration, row++, 0, 1, 6);
    m_gridLayout->addWidget(m_fileType, row++, 0, 1, 6);
    m_gridLayout->addWidget(m_fileAccessTime, row++, 0, 1, 6);
    m_gridLayout->addWidget(m_fileModifyTime, row++, 0, 1, 6);

    setLayout(m_gridLayout);
}

void FileInfoWidget::initConnections()
{
    // Establish signal connections once - never disconnect
    connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished,
            this, &FileInfoWidget::onImageMediaInfo);
    connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished,
            this, &FileInfoWidget::onVideoMediaInfo);
    connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished,
            this, &FileInfoWidget::onAudioMediaInfo);
}

void FileInfoWidget::setUrl(const QUrl &url)
{
    m_currentUrl = url;

    // Step 1: Reset all fields to default visible state with empty values
    resetAllFields();

    // Step 2: Apply field filters (hide filtered fields)
    applyFieldFilters(url);

    // Step 3: Apply field expansions (replace values, insert dynamic fields)
    applyFieldExpansions(url);

    // Step 4: Fill actual values from file info
    fillFieldValues(url);

    adjustSize();
}

void FileInfoWidget::resetAllFields()
{
    // Reset all core fields to visible with empty right values
    auto resetField = [](KeyValueLabel *field) {
        if (field) {
            field->setRightValue(QString(), Qt::ElideNone, Qt::AlignLeft, false);
            field->setVisible(true);
        }
    };

    resetField(m_fileName);
    resetField(m_fileSize);
    resetField(m_fileViewSize);
    resetField(m_fileDuration);
    resetField(m_fileType);
    resetField(m_fileAccessTime);
    resetField(m_fileModifyTime);

    // Delete any dynamic expansion fields from previous URL
    for (KeyValueLabel *field : m_dynamicFields) {
        m_gridLayout->removeWidget(field);
        field->deleteLater();
    }
    m_dynamicFields.clear();

    // Rebuild field map without dynamic fields
    m_fieldMap.clear();
    m_fieldMap.insert(BasicFieldExpandEnum::kFileName, m_fileName);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileSize, m_fileSize);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileViewSize, m_fileViewSize);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileDuration, m_fileDuration);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileType, m_fileType);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileInterviewTime, m_fileAccessTime);
    m_fieldMap.insert(BasicFieldExpandEnum::kFileChangeTIme, m_fileModifyTime);
}

void FileInfoWidget::applyFieldFilters(const QUrl &url)
{
    QUrl filterUrl = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ filterUrl }, &urls);

    if (ok && !urls.isEmpty())
        filterUrl = urls.first();

    DetailFilterType fieldFilters = DetailManager::instance().basicFiledFiltes(filterUrl);

    // Use setVisible(false) instead of deleteLater() - key optimization!
    if (fieldFilters & DetailFilterType::kFileNameField)
        m_fileName->setVisible(false);

    if (fieldFilters & DetailFilterType::kFileSizeField)
        m_fileSize->setVisible(false);

    if (fieldFilters & DetailFilterType::kFileViewSizeField)
        m_fileViewSize->setVisible(false);

    if (fieldFilters & DetailFilterType::kFileDurationField)
        m_fileDuration->setVisible(false);

    if (fieldFilters & DetailFilterType::kFileTypeField)
        m_fileType->setVisible(false);

    if (fieldFilters & DetailFilterType::kFileInterviewTimeField)
        m_fileAccessTime->setVisible(false);

    if (fieldFilters & DetailFilterType::kFileChangeTimeField)
        m_fileModifyTime->setVisible(false);
}

void FileInfoWidget::applyFieldExpansions(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> fieldCondition =
            DetailManager::instance().createBasicViewExtensionField(url);

    for (auto it = fieldCondition.constBegin(); it != fieldCondition.constEnd(); ++it) {
        BasicExpandType expandType = it.key();
        const BasicExpandMap &expandMap = it.value();

        switch (expandType) {
        case kFieldInsert: {
            // Insert new dynamic fields
            for (auto enumIt = expandMap.constBegin(); enumIt != expandMap.constEnd(); ++enumIt) {
                BasicFieldExpandEnum fieldEnum = enumIt.key();
                const QPair<QString, QString> &field = enumIt.value();

                KeyValueLabel *expandLabel = new KeyValueLabel(this);
                expandLabel->setLeftValue(field.first);
                expandLabel->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::DemiBold);
                expandLabel->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignLeft, true);
                expandLabel->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);

                m_fieldMap.insert(fieldEnum, expandLabel);
                m_dynamicFields.append(expandLabel);

                // Insert after the corresponding core field
                int insertRow = static_cast<int>(fieldEnum);
                m_gridLayout->addWidget(expandLabel, insertRow, 0, 1, 6);
            }
        } break;

        case kFieldReplace: {
            // Replace values in existing fields
            for (auto enumIt = expandMap.constBegin(); enumIt != expandMap.constEnd(); ++enumIt) {
                BasicFieldExpandEnum fieldEnum = enumIt.key();
                const QPair<QString, QString> &field = enumIt.value();

                KeyValueLabel *targetField = m_fieldMap.value(fieldEnum);
                if (targetField) {
                    targetField->setLeftValue(field.first);
                    targetField->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignVCenter, true);
                }
            }
        } break;
        }
    }
}

void FileInfoWidget::fillFieldValues(const QUrl &url)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull()) {
        fmWarning() << "Failed to create file info for URL:" << url.toString();
        return;
    }

    // Fill name (if not already set by expansion)
    if (m_fileName->isVisible() && m_fileName->RightValue().isEmpty()) {
        m_fileName->setRightValue(info->displayOf(DisPlayInfoType::kFileDisplayName),
                                  Qt::ElideMiddle, Qt::AlignLeft, true);
    }

    // Fill access time
    if (m_fileAccessTime->isVisible() && m_fileAccessTime->RightValue().isEmpty()) {
        auto lastRead = info->timeOf(TimeInfoType::kLastRead).value<QDateTime>();
        if (lastRead.isValid()) {
            m_fileAccessTime->setRightValue(formatDateTime(lastRead),
                                            Qt::ElideMiddle, Qt::AlignLeft, true, 150);
        } else {
            m_fileAccessTime->setVisible(false);
        }
    }

    // Fill modify time
    if (m_fileModifyTime->isVisible() && m_fileModifyTime->RightValue().isEmpty()) {
        auto lastModified = info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
        if (lastModified.isValid()) {
            m_fileModifyTime->setRightValue(formatDateTime(lastModified),
                                            Qt::ElideMiddle, Qt::AlignLeft, true, 150);
        } else {
            m_fileModifyTime->setVisible(false);
        }
    }

    // Fill file size (only for non-directories)
    if (m_fileSize->isVisible()) {
        m_fileSize->setVisible(false);   // Default hidden
        if (!info->isAttributes(OptInfoType::kIsDir) && m_fileSize->RightValue().isEmpty()) {
            m_fileSize->setVisible(true);
            m_fileSize->setRightValue(FileUtils::formatSize(info->size()),
                                      Qt::ElideNone, Qt::AlignLeft, true);
            m_fileSize->adjustHeight();
        }
    }

    // Hide resolution and duration by default (will be shown by media info callbacks)
    if (m_fileViewSize->isVisible() && m_fileViewSize->RightValue().isEmpty())
        m_fileViewSize->setVisible(false);

    if (m_fileDuration->isVisible() && m_fileDuration->RightValue().isEmpty())
        m_fileDuration->setVisible(false);

    // Get local URL for media type detection
    QUrl localUrl = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    FileInfoPointer localInfo = InfoFactory::create<FileInfo>(localUrl);

    // Fill file type and request media info
    if (m_fileType->isVisible() && m_fileType->RightValue().isEmpty() && localInfo) {
        const QString &mimeName = localInfo->nameOf(NameInfoType::kMimeTypeName);
        const FileInfo::FileType type = MimeTypeDisplayManager::instance()->displayNameToEnum(mimeName);
        m_fileType->setRightValue(localInfo->displayOf(DisPlayInfoType::kMimeTypeDisplayName),
                                  Qt::ElideNone, Qt::AlignLeft, true);

        // Request media attributes based on file type
        QList<DFileInfo::AttributeExtendID> extenList;
        if (type == FileInfo::FileType::kVideos) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth
                      << DFileInfo::AttributeExtendID::kExtendMediaHeight
                      << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            const QMap<DFileInfo::AttributeExtendID, QVariant> &mediaAttributes =
                    localInfo->mediaInfoAttributes(DFileInfo::MediaType::kVideo, extenList);
            if (!mediaAttributes.isEmpty())
                onVideoMediaInfo(url, mediaAttributes);
        } else if (type == FileInfo::FileType::kImages) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth
                      << DFileInfo::AttributeExtendID::kExtendMediaHeight;
            const QMap<DFileInfo::AttributeExtendID, QVariant> &mediaAttributes =
                    localInfo->mediaInfoAttributes(DFileInfo::MediaType::kImage, extenList);
            if (!mediaAttributes.isEmpty())
                onImageMediaInfo(url, mediaAttributes);
        } else if (type == FileInfo::FileType::kAudios) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            const QMap<DFileInfo::AttributeExtendID, QVariant> &mediaAttributes =
                    localInfo->mediaInfoAttributes(DFileInfo::MediaType::kAudio, extenList);
            if (!mediaAttributes.isEmpty())
                onAudioMediaInfo(url, mediaAttributes);
        }
    }
}

QString FileInfoWidget::formatDateTime(const QDateTime &time) const
{
    if (time.toMSecsSinceEpoch() == 0)
        return QStringLiteral("-");

    return time.toString(FileUtils::dateTimeFormat());
}

void FileInfoWidget::onImageMediaInfo(const QUrl &url, QMap<DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url != m_currentUrl || properties.isEmpty())
        return;

    int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
    int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();

    if (width <= 0 || height <= 0)
        return;

    if (m_fileViewSize && m_fileViewSize->RightValue().isEmpty()) {
        QString viewSize = QString::number(width) + "x" + QString::number(height);
        m_fileViewSize->setVisible(true);
        m_fileViewSize->setRightValue(viewSize, Qt::ElideNone, Qt::AlignLeft, true);
        m_fileViewSize->adjustHeight();
    }
}

void FileInfoWidget::onVideoMediaInfo(const QUrl &url, QMap<DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url != m_currentUrl || properties.isEmpty())
        return;

    int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
    int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();

    if (width > 0 && height > 0 && m_fileViewSize && m_fileViewSize->RightValue().isEmpty()) {
        QString viewSize = QString::number(width) + "x" + QString::number(height);
        m_fileViewSize->setVisible(true);
        m_fileViewSize->setRightValue(viewSize, Qt::ElideNone, Qt::AlignLeft, true);
        m_fileViewSize->adjustHeight();
    }

    int durationMs = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration].toInt();
    if (durationMs > 0 && m_fileDuration && m_fileDuration->RightValue().isEmpty()) {
        QTime t(0, 0, 0);
        t = t.addMSecs(durationMs);
        QString durationStr = t.toString("hh:mm:ss");
        m_fileDuration->setVisible(true);
        m_fileDuration->setRightValue(durationStr, Qt::ElideNone, Qt::AlignLeft, true);
        m_fileDuration->adjustHeight();
    }
}

void FileInfoWidget::onAudioMediaInfo(const QUrl &url, QMap<DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url != m_currentUrl || properties.isEmpty())
        return;

    int durationMs = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration].toInt();
    if (durationMs > 0 && m_fileDuration && m_fileDuration->RightValue().isEmpty()) {
        QTime t(0, 0, 0);
        t = t.addMSecs(durationMs);
        QString durationStr = t.toString("hh:mm:ss");
        m_fileDuration->setVisible(true);
        m_fileDuration->setRightValue(durationStr, Qt::ElideNone, Qt::AlignLeft, true);
        m_fileDuration->adjustHeight();
    }
}
