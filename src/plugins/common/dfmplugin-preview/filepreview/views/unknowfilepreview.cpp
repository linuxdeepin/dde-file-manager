// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unknowfilepreview.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"

#include "dfm-base/base/schemefactory.h"

#include <QVBoxLayout>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

UnknowFilePreview::UnknowFilePreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    contentView = new QWidget();
    contentView->setFixedSize(590, 260);
    iconLabel = new QLabel(contentView);
    iconLabel->setObjectName("IconLabel");
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nameLabel = new QLabel(contentView);
    nameLabel->setObjectName("NameLabel");
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    nameLabel->setWordWrap(true);
    QFont font;
    font.setWeight(QFont::DemiBold);
    font.setPointSize(12);
    nameLabel->setFont(font);
    sizeLabel = new QLabel(contentView);
    sizeLabel->setObjectName("SizeLabel");
    sizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    typeLabel = new QLabel(contentView);
    typeLabel->setObjectName("TypeLabel");

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

    fileCalculationUtils = new FileStatisticsJob;
    connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &UnknowFilePreview::updateFolderSizeCount);
}

UnknowFilePreview::~UnknowFilePreview()
{
    if (contentView) {
        contentView->deleteLater();
    }

    if (fileCalculationUtils) {
        fileCalculationUtils->deleteLater();
    }
}

bool UnknowFilePreview::setFileUrl(const QUrl &url)
{
    this->url = url;
    const AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

    if (!info.isNull()) {
        setFileInfo(info);
        return true;
    }

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

void UnknowFilePreview::setFileInfo(const AbstractFileInfoPointer &info)
{
    const QIcon &icon = info->fileIcon();

    iconLabel->setPixmap(icon.pixmap(180));

    QFont font = nameLabel->font();
    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(info->nameOf(NameInfoType::kFileName), Qt::ElideMiddle, 800);

    nameLabel->setText(elidedText);

    if (info->isAttributes(OptInfoType::kIsFile) || info->isAttributes(OptInfoType::kIsSymLink)) {
        sizeLabel->setText(QObject::tr("Size: %1").arg(info->displayOf(DisPlayInfoType::kSizeDisplayName)));
        typeLabel->setText(QObject::tr("Type: %1").arg(info->displayOf(DisPlayInfoType::kMimeTypeDisplayName)));
    } else if (info->isAttributes(OptInfoType::kIsDir)) {
        fileCalculationUtils->start(QList<QUrl>() << info->urlOf(UrlInfoType::kUrl));
        sizeLabel->setText(QObject::tr("Size: 0"));
    }
}

void UnknowFilePreview::updateFolderSizeCount(qint64 size, int filesCount, int directoryCount)
{
    sizeLabel->setText(QObject::tr("Size: %1").arg(FileUtils::formatSize(size)));
    typeLabel->setText(QObject::tr("Items: %1").arg(filesCount + directoryCount));
}
