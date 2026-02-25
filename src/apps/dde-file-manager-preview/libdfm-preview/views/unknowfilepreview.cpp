// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unknowfilepreview.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/elidetextlayout.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>
#include <dfm-base/base/schemefactory.h>

#include <QVBoxLayout>
#include <QTextDocument>
#include <QTextBlock>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)

UnknowFilePreview::UnknowFilePreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    qCDebug(logLibFilePreview) << "UnknowFilePreview: initializing unknown file preview widget";

    contentView = new QWidget();
    contentView->setFixedSize(590, 260);
    iconLabel = new QLabel(contentView);
    iconLabel->setObjectName("IconLabel");
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nameLabel = new QLabel(contentView);
    nameLabel->setObjectName("NameLabel");
    nameLabel->setTextFormat(Qt::PlainText);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QFont font;
    font.setWeight(QFont::DemiBold);
    font.setPointSize(12);
    nameLabel->setFont(font);
    sizeLabel = new QLabel(contentView);
    sizeLabel->setObjectName("SizeLabel");
    sizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    typeLabel = new QLabel(contentView);
    typeLabel->setObjectName("TypeLabel");
    typeLabel->setWordWrap(true);
    typeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    typeLabel->adjustSize();
    QVBoxLayout *vlayout = new QVBoxLayout();

    vlayout->setContentsMargins(0, 16, 0, 0);
    vlayout->addWidget(nameLabel);
    vlayout->addSpacing(10);
    vlayout->addWidget(sizeLabel);
    vlayout->addWidget(typeLabel);
    vlayout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout(contentView);

    hlayout->setContentsMargins(60, 60, 20, 20);
    hlayout->addWidget(iconLabel);
    hlayout->addSpacing(30);
    hlayout->addLayout(vlayout);
    hlayout->addStretch();

    fileCalculationUtils = new FileScanner(this);
    connect(fileCalculationUtils, &FileScanner::progressChanged, this, &UnknowFilePreview::updateFolderSizeCount);

    qCDebug(logLibFilePreview) << "UnknowFilePreview: initialization completed";
}

UnknowFilePreview::~UnknowFilePreview()
{
    qCDebug(logLibFilePreview) << "UnknowFilePreview: destructor called";

    if (contentView) {
        contentView->deleteLater();
    }

    if (fileCalculationUtils) {
        fileCalculationUtils->deleteLater();
    }
}

bool UnknowFilePreview::setFileUrl(const QUrl &url)
{
    qCInfo(logLibFilePreview) << "UnknowFilePreview: setting file URL:" << url.toString();

    this->url = url;
    const FileInfoPointer info = InfoFactory::create<FileInfo>(url);

    if (!info.isNull()) {
        setFileInfo(info);
        qCInfo(logLibFilePreview) << "UnknowFilePreview: successfully set file URL and info";
        return true;
    }

    qCWarning(logLibFilePreview) << "UnknowFilePreview: failed to create file info for URL:" << url.toString();
    return false;
}

QUrl UnknowFilePreview::fileUrl() const
{
    return url;
}

QWidget *UnknowFilePreview::contentWidget() const
{
    return contentView;
}

void UnknowFilePreview::setFileInfo(const FileInfoPointer &info)
{
    if (!info) {
        qCWarning(logLibFilePreview) << "UnknowFilePreview: null file info provided";
        return;
    }

    qCDebug(logLibFilePreview) << "UnknowFilePreview: setting file info for:" << info->nameOf(NameInfoType::kFileName);

    if (fileCalculationUtils) {
        fileCalculationUtils->stop();
    }

    QIcon icon;
    ThumbnailHelper helper;
    if (helper.checkThumbEnable(url)) {
        icon = info->extendAttributes(ExtInfoType::kFileThumbnail).value<QIcon>();
        if (icon.isNull()) {
            const auto &img = helper.thumbnailImage(url, Global::kLarge);
            icon = QPixmap::fromImage(img);
            if (!icon.isNull())
                info->setExtendedAttributes(ExtInfoType::kFileThumbnail, icon);
            else
                icon = info->fileIcon();
        }
    } else {
        icon = info->fileIcon();
    }
    iconLabel->setPixmap(icon.pixmap(180));

    QFont font = nameLabel->font();
    QRect rect(QPoint(0, 0), QSize(240, 80));
    QStringList labelTexts;
    ElideTextLayout layout(info->nameOf(NameInfoType::kFileName));
    layout.documentHandle()->firstBlock().layout()->setFont(font);
    layout.layout(rect, Qt::ElideMiddle, nullptr, Qt::NoBrush, &labelTexts);
    const QString &elidedText = labelTexts.join('\n');
    nameLabel->setText(elidedText);

    if (info->isAttributes(OptInfoType::kIsFile) || info->isAttributes(OptInfoType::kIsSymLink)) {
        QString sizeText = info->displayOf(DisPlayInfoType::kSizeDisplayName);
        QString typeText = info->displayOf(DisPlayInfoType::kMimeTypeDisplayName);

        sizeLabel->setText(QObject::tr("Size: %1").arg(sizeText));
        typeLabel->setText(QObject::tr("Type: %1").arg(typeText));

        qCDebug(logLibFilePreview) << "UnknowFilePreview: file info set - size:" << sizeText << "type:" << typeText;
    } else if (fileCalculationUtils && info->isAttributes(OptInfoType::kIsDir)) {
        qCInfo(logLibFilePreview) << "UnknowFilePreview: starting directory size calculation for:" << info->urlOf(UrlInfoType::kUrl).toString();
        fileCalculationUtils->start(QList<QUrl>() << info->urlOf(UrlInfoType::kUrl));
        sizeLabel->setText(QObject::tr("Size: 0"));
    }
}

void UnknowFilePreview::updateFolderSizeCount(const FileScanner::ScanResult &result)
{
    QString sizeText = FileUtils::formatSize(result.totalSize);
    int totalItems = result.fileCount + result.directoryCount;

    sizeLabel->setText(QObject::tr("Size: %1").arg(sizeText));
    typeLabel->setText(QObject::tr("Items: %1").arg(totalItems));
}
