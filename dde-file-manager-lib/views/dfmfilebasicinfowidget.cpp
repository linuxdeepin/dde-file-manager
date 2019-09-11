/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#include "dfmfilebasicinfowidget.h"
#include "dfmeventdispatcher.h"
#include "dfileinfo.h"
#include "dfileservices.h"
#include "models/trashfileinfo.h"
#include "dfilestatisticsjob.h"
#include "shutil/fileutils.h"
#include "app/define.h"
#include "singleton.h"
#include "shutil/mimetypedisplaymanager.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QStackedLayout>

DFM_BEGIN_NAMESPACE

SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionKeyLabel");
    setFixedWidth(120);
    setAlignment(Qt::AlignVCenter | Qt::AlignRight);
}


SectionValueLabel::SectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionValueLabel");
    setFixedWidth(150);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setWordWrap(true);
}

LinkSectionValueLabel::LinkSectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    SectionValueLabel(text, parent, f)
{

}

void LinkSectionValueLabel::mouseReleaseEvent(QMouseEvent *event)
{
    DFMEventDispatcher::instance()->processEvent<DFMOpenFileLocation>(Q_NULLPTR, linkTargetUrl());
    SectionValueLabel::mouseReleaseEvent(event);
}

DUrl LinkSectionValueLabel::linkTargetUrl() const
{
    return m_linkTargetUrl;
}

void LinkSectionValueLabel::setLinkTargetUrl(const DUrl &linkTargetUrl)
{
    m_linkTargetUrl = linkTargetUrl;
}

class DFMFileBasicInfoWidgetPrivate
{
public:
    DFMFileBasicInfoWidgetPrivate(DFMFileBasicInfoWidget *qq);
    ~DFMFileBasicInfoWidgetPrivate();

    void setUrl(const DUrl &url);
protected:
    void initUI();
    void startComputerFolderSize(const DUrl &url);
    void update();

private:
    DUrl        m_url;
    QLabel      *m_folderSizeLabel{ nullptr };
    QLabel      *m_containSizeLabel{ nullptr };
    bool         m_showFileName{ false };
    bool         m_showPicturePixel{ false };
    bool         m_showVideoInfo{ false }; // 视频时长

    DFM_NAMESPACE::DFileStatisticsJob* m_sizeWorker{ nullptr };

    DFMFileBasicInfoWidget *q_ptr;
    Q_DECLARE_PUBLIC(DFMFileBasicInfoWidget)
};

DFMFileBasicInfoWidgetPrivate::DFMFileBasicInfoWidgetPrivate(DFMFileBasicInfoWidget *qq)
    :q_ptr(qq)
{
    initUI();
}

DFMFileBasicInfoWidgetPrivate::~DFMFileBasicInfoWidgetPrivate()
{
    if (m_sizeWorker)
        m_sizeWorker->stop();
}

void DFMFileBasicInfoWidgetPrivate::startComputerFolderSize(const DUrl &url)
{
    Q_Q(DFMFileBasicInfoWidget);

    DUrl validUrl = url;
    if (url.isUserShareFile()) {
        validUrl.setScheme(FILE_SCHEME);
    }

    DUrlList urls;
    urls << validUrl;

    if (!m_sizeWorker) {
        m_sizeWorker = new DFileStatisticsJob(q);

        QObject::connect(m_sizeWorker, &DFileStatisticsJob::dataNotify, q,
                [=](qint64 size, int filesCount, int directoryCount){
            int m_fileCount = filesCount + directoryCount;
            if (m_folderSizeLabel)
                m_folderSizeLabel->setText(FileUtils::formatSize(size));
            if (m_containSizeLabel)
                m_containSizeLabel->setText(QString::number(m_fileCount));
        });
    }

    m_sizeWorker->start(urls);
}

void DFMFileBasicInfoWidgetPrivate::initUI()
{
    Q_Q(DFMFileBasicInfoWidget);
    q->setLayout(new QStackedLayout);

}
void DFMFileBasicInfoWidgetPrivate::setUrl(const DUrl &url)
{
    m_url = url;
    Q_Q(DFMFileBasicInfoWidget);
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(q, m_url);
    if (!info)
        return;

     QStackedLayout *stackedLayout = qobject_cast<QStackedLayout*>(q->layout());
     if (!stackedLayout)
         return;

    if (stackedLayout->currentWidget()) {
        if (m_sizeWorker)
            m_sizeWorker->stop();
        delete stackedLayout->currentWidget();
    }


    QWidget *layoutWidget = new QWidget;
    QFormLayout *layout = new QFormLayout(layoutWidget);
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(16);
    layout->setLabelAlignment(Qt::AlignRight);
    stackedLayout->addWidget(layoutWidget);
    stackedLayout->setCurrentWidget(layoutWidget);

    int frameHeight = 160;
    if (m_showFileName){
        QLabel *fileNameKeyLabel = new SectionKeyLabel(QObject::tr("File name"));
        QLabel *fileNameLabel = new SectionValueLabel(info->fileDisplayName());
        QString text = info->fileDisplayName();
        fileNameLabel->setText(fileNameLabel->fontMetrics().elidedText(text, Qt::ElideMiddle, fileNameLabel->width()));
        fileNameLabel->setToolTip(text);

        frameHeight += 30;
        layout->addRow(fileNameKeyLabel, fileNameLabel);
    }

    SectionKeyLabel *sizeSectionLabel = new SectionKeyLabel(QObject::tr("Size"));
    SectionKeyLabel *typeSectionLabel = new SectionKeyLabel(QObject::tr("Type"));
    SectionKeyLabel *TimeCreatedSectionLabel = new SectionKeyLabel(QObject::tr("Time read"));
    SectionKeyLabel *TimeModifiedSectionLabel = new SectionKeyLabel(QObject::tr("Time modified"));
    SectionKeyLabel *sourcePathSectionLabel = new SectionKeyLabel(QObject::tr("Source path"));

    m_containSizeLabel = new SectionValueLabel(info->sizeDisplayName());
    m_folderSizeLabel = new SectionValueLabel;
    SectionValueLabel *typeLabel = new SectionValueLabel(info->mimeTypeDisplayName());
    SectionValueLabel *timeCreatedLabel = new SectionValueLabel(info->lastReadDisplayName());
    SectionValueLabel *timeModifiedLabel = new SectionValueLabel(info->lastModifiedDisplayName());

    if (info->isDir()) {
        SectionKeyLabel *fileAmountSectionLabel = new SectionKeyLabel(QObject::tr("Contains"));
        layout->addRow(sizeSectionLabel, m_folderSizeLabel);
        layout->addRow(fileAmountSectionLabel, m_containSizeLabel);

        if (info->toLocalFile().isEmpty()) {
            startComputerFolderSize(m_url);
        } else if (info->isSymLink()) {
            startComputerFolderSize(info->redirectedFileUrl());
        } else {
            startComputerFolderSize(DUrl::fromLocalFile(info->toLocalFile()));
        }

        frameHeight += 30;
    } else {
        layout->addRow(sizeSectionLabel, m_containSizeLabel);
    }

    if (m_showPicturePixel) {
        DAbstractFileInfo::FileType fileType = mimeTypeDisplayManager->displayNameToEnum(info->mimeTypeName());
        if (fileType == DAbstractFileInfo::FileType::Images) {
            QPixmap pixmap;
            pixmap.load(info->filePath());
            if (!pixmap.isNull()) {
                QString text = QString("%1X%2").arg(pixmap.width()).arg(pixmap.height());

                QLabel *pixelKeyLabel = new SectionKeyLabel(QObject::tr("Picture size"));
                QLabel *pixelLabel = new SectionValueLabel;
                pixelLabel->setText(text);
                layout->addRow(pixelKeyLabel, pixelLabel);
                frameHeight += 30;
            }
        }
    }

    if (m_showVideoInfo) {
        DAbstractFileInfo::FileType fileType = mimeTypeDisplayManager->displayNameToEnum(info->mimeTypeName());
        if (fileType == DAbstractFileInfo::FileType::Videos) {
//            QSharedPointer<VideoWidget> playerWidget = QSharedPointer<VideoWidget>(new VideoWidget);
//            QPixmap pixmap;
//            pixmap.load(info->filePath());
//            if (!pixmap.isNull()) {
//                QString text = QString("%1X%2").arg(pixmap.width()).arg(pixmap.height());

//                QLabel *pixelKeyLabel = new SectionKeyLabel(QObject::tr("Picture size"));
//                QLabel *pixelLabel = new SectionValueLabel;
//                pixelLabel->setText(text);
//                layout->addRow(pixelKeyLabel, pixelLabel);
//                frameHeight += 30;
//            }
        }
    }

    if (!info->isVirtualEntry()) {
        layout->addRow(typeSectionLabel, typeLabel);
    }

    if (info->isSymLink()) {
        SectionKeyLabel *linkPathSectionLabel = new SectionKeyLabel(QObject::tr("Link path"));

        LinkSectionValueLabel *linkPathLabel = new LinkSectionValueLabel(info->symlinkTargetPath());
        linkPathLabel->setToolTip(info->symlinkTargetPath());
        linkPathLabel->setLinkTargetUrl(info->redirectedFileUrl());
        linkPathLabel->setOpenExternalLinks(true);
        linkPathLabel->setWordWrap(false);
        QString t = linkPathLabel->fontMetrics().elidedText(info->symlinkTargetPath(), Qt::ElideMiddle, 150);
        linkPathLabel->setText(t);
        layout->addRow(linkPathSectionLabel, linkPathLabel);
        frameHeight += 30;
    }

    if (!info->isVirtualEntry()) {
        layout->addRow(TimeCreatedSectionLabel, timeCreatedLabel);
        layout->addRow(TimeModifiedSectionLabel, timeModifiedLabel);
    }

    if (info->fileUrl().isTrashFile()) {
        QString pathStr = static_cast<const TrashFileInfo *>(info.constData())->sourceFilePath();
        SectionValueLabel *sourcePathLabel = new SectionValueLabel(pathStr);
        QString elidedStr = sourcePathLabel->fontMetrics().elidedText(pathStr, Qt::ElideMiddle, 150);
        sourcePathLabel->setToolTip(pathStr);
        sourcePathLabel->setWordWrap(false);
        sourcePathLabel->setText(elidedStr);
        layout->addRow(sourcePathSectionLabel, sourcePathLabel);
    }

    layout->setContentsMargins(0, 0, 40, 0);
    q->setFixedHeight(frameHeight);
}

DFMFileBasicInfoWidget::DFMFileBasicInfoWidget(QWidget *parent)
    : QFrame (parent)
    , d_private(new DFMFileBasicInfoWidgetPrivate(this))
{

}

DFMFileBasicInfoWidget::~DFMFileBasicInfoWidget()
{

}

void DFMFileBasicInfoWidget::setUrl(const DUrl &url)
{
    Q_D(DFMFileBasicInfoWidget);
    d->setUrl(url);
}

bool DFMFileBasicInfoWidget::showFileName()
{
    Q_D(DFMFileBasicInfoWidget);
    return d->m_showFileName;
}

void DFMFileBasicInfoWidget::setShowFileName(bool visible)
{
    Q_D(DFMFileBasicInfoWidget);
    d->m_showFileName = visible;
}

bool DFMFileBasicInfoWidget::showPicturePixel()
{
    Q_D(DFMFileBasicInfoWidget);
    return d->m_showPicturePixel;
}

void DFMFileBasicInfoWidget::setShowPicturePixel(bool visible)
{
    Q_D(DFMFileBasicInfoWidget);
    d->m_showPicturePixel = visible;
}

bool DFMFileBasicInfoWidget::showVideoInfo()
{
    Q_D(DFMFileBasicInfoWidget);
    return d->m_showVideoInfo;
}

void DFMFileBasicInfoWidget::setShowVideoInfo(bool visible)
{
    Q_D(DFMFileBasicInfoWidget);
    d->m_showVideoInfo = visible;
}

DFM_END_NAMESPACE
