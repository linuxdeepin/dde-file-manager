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
#include "unknowfilepreview.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"

#include "dfm-base/base/schemefactory.h"

#include <QVBoxLayout>

DFMBASE_USE_NAMESPACE
DPFILEPREVIEW_USE_NAMESPACE

UnknowFilePreview::UnknowFilePreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    contentView = new QWidget();
    contentView->setFixedSize(550, 200);
    iconLabel = new QLabel(contentView);
    iconLabel->setObjectName("IconLabel");
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nameLabel = new QLabel(contentView);
    nameLabel->setObjectName("NameLabel");
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setWordWrap(true);
    sizeLabel = new QLabel(contentView);
    sizeLabel->setObjectName("SizeLabel");
    sizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    typeLabel = new QLabel(contentView);
    typeLabel->setObjectName("TypeLabel");
    typeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QVBoxLayout *vlayout = new QVBoxLayout();

    vlayout->addWidget(nameLabel);
    vlayout->addWidget(sizeLabel);
    vlayout->addWidget(typeLabel);
    vlayout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout(contentView);

    hlayout->setContentsMargins(30, 20, 20, 20);
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

    iconLabel->setPixmap(icon.pixmap(150));

    QFont font = nameLabel->font();
    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(info->fileName(), Qt::ElideMiddle, 300);

    nameLabel->setText(elidedText);

    if (info->isDir()) {
        fileCalculationUtils->start(QList<QUrl>() << info->url());
        sizeLabel->setText(QObject::tr("Size: 0"));
    } else if (info->isFile()) {
        sizeLabel->setText(QObject::tr("Size: %1").arg(info->sizeDisplayName()));
        QMimeType mimeType = MimeDatabase::mimeTypeForUrl(url);
        QString mimeTypeDisplayName = MimeDatabase::mimeFileType(mimeType.name());
        typeLabel->setText(QObject::tr("Type: %1").arg(mimeTypeDisplayName));
    }
}

void UnknowFilePreview::updateFolderSizeCount(qint64 size, int filesCount, int directoryCount)
{
    sizeLabel->setText(QObject::tr("Size: %1").arg(FileUtils::formatSize(size)));
    typeLabel->setText(QObject::tr("Items: %1").arg(filesCount + directoryCount));
}
