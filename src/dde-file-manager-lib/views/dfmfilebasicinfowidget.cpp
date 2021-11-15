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
#include "app/filesignalmanager.h"
#include "singleton.h"
#include "shutil/mimetypedisplaymanager.h"
#include "controllers/vaultcontroller.h"
#include "dfmsettings.h"
#include "vaultglobaldefine.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QStackedLayout>
#include <QTimer>

#include <mediainfo/dfmmediainfo.h>

DFM_BEGIN_NAMESPACE

SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionKeyLabel");
    setFixedWidth(100);
    QFont font = this->font();
    font.setWeight(QFont::Bold - 8);
    font.setPixelSize(13);
    setFont(font);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}


SectionValueLabel::SectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionValueLabel");
    setFixedWidth(150);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

bool SectionValueLabel::event(QEvent *e)
{
    if (e->type() == QEvent::FontChange) {
        //! 根据字体大小来设置label的宽度
        setFixedWidth(this->fontMetrics().horizontalAdvance(this->text()));
    }
    return QLabel::event(e);
}

void SectionValueLabel::showEvent(QShowEvent *e)
{
    QString txt = this->text();
    if (!txt.isEmpty()) {
        //! 根据字体大小来设置label的宽度
        setFixedWidth(this->fontMetrics().horizontalAdvance(txt));
    }
    QLabel::showEvent(e);
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
    explicit DFMFileBasicInfoWidgetPrivate(DFMFileBasicInfoWidget *qq);
    ~DFMFileBasicInfoWidgetPrivate();

    void setUrl(const DUrl &url);
protected:
    void initUI();
    void startCalcFolderSize();
    void update();

private:
    DUrl        m_url;
    QLabel      *m_folderSizeLabel{ nullptr };
    QLabel      *m_containSizeLabel{ nullptr };
    bool         m_showFileName{ false };
    bool         m_showMediaInfo{ false };
    bool         m_showSummaryOnly{ false };

    DFM_NAMESPACE::DFileStatisticsJob *m_sizeWorker{ nullptr };

    DFMFileBasicInfoWidget *q_ptr;
    Q_DECLARE_PUBLIC(DFMFileBasicInfoWidget)
};

DFMFileBasicInfoWidgetPrivate::DFMFileBasicInfoWidgetPrivate(DFMFileBasicInfoWidget *qq)
    : q_ptr(qq)
{
    initUI();
}

DFMFileBasicInfoWidgetPrivate::~DFMFileBasicInfoWidgetPrivate()
{
    if (m_sizeWorker)
        m_sizeWorker->stop();
}

void DFMFileBasicInfoWidgetPrivate::startCalcFolderSize()
{
    Q_Q(DFMFileBasicInfoWidget);
    if (m_showSummaryOnly)
        return;

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(q, m_url);
    if (!info)
        return;
    DUrl validUrl = m_url;
    if (info->isSymLink()) {
        validUrl = info->redirectedFileUrl();
    } else {
        validUrl = DUrl::fromLocalFile(info->toLocalFile());
    }

    if (validUrl.isUserShareFile()) {
        validUrl.setScheme(FILE_SCHEME);
    }

    DUrlList urls;
    urls << validUrl;

    if (!m_sizeWorker) {
        m_sizeWorker = new DFileStatisticsJob(q);
        QObject::connect(m_sizeWorker, &DFileStatisticsJob::dataNotify, q, &DFMFileBasicInfoWidget::updateSizeText);
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
    if (m_url == url) {
        return;
    }

    m_url = url;
    Q_Q(DFMFileBasicInfoWidget);
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(q, m_url);
    if (!info)
        return;

    QStackedLayout *stackedLayout = qobject_cast<QStackedLayout *>(q->layout());
    if (!stackedLayout)
        return;

    if (stackedLayout->currentWidget()) {
        if (m_sizeWorker)
            m_sizeWorker->stop();
        delete stackedLayout->currentWidget();
    }

    QWidget *layoutWidget = new QWidget;
    QFormLayout *layout = new QFormLayout(layoutWidget);
    layout->setHorizontalSpacing(5);
    layout->setVerticalSpacing(16);
    layout->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    stackedLayout->addWidget(layoutWidget);
    stackedLayout->setCurrentWidget(layoutWidget);

    if (m_showFileName) {
        QLabel *fileNameKeyLabel = new SectionKeyLabel(QObject::tr("Name"));
        QLabel *fileNameLabel = new SectionValueLabel(info->fileDisplayName());
        QString text = info->fileDisplayName();
        fileNameLabel->setText(fileNameLabel->fontMetrics().elidedText(text, Qt::ElideMiddle, fileNameLabel->width()));
        fileNameLabel->setToolTip(text);

        layout->addRow(fileNameKeyLabel, fileNameLabel);
    }

    m_folderSizeLabel = new SectionValueLabel("", layoutWidget);
    SectionValueLabel *typeLabel = new SectionValueLabel(info->mimeTypeDisplayName().split(" (")[0]);

    if (info->isDir()) {
        if (!m_showSummaryOnly) {
            m_containSizeLabel = new SectionValueLabel(info->sizeDisplayName());
            SectionKeyLabel *sizeSectionLabel = new SectionKeyLabel(QObject::tr("Size"));
            SectionKeyLabel *fileAmountSectionLabel = new SectionKeyLabel(QObject::tr("Contains"));
            layout->addRow(sizeSectionLabel, m_folderSizeLabel);
            layout->addRow(fileAmountSectionLabel, m_containSizeLabel);
        }
    } else {
        SectionKeyLabel *sizeSectionLabel = new SectionKeyLabel(QObject::tr("Size"));
        layout->addRow(sizeSectionLabel, m_folderSizeLabel);
    }

    if (m_showMediaInfo) {
        DAbstractFileInfo::FileType fileType = mimeTypeDisplayManager->displayNameToEnum(info->mimeTypeName());

        DFMMediaInfo::MeidiaType mediaType = DFMMediaInfo::Other;
        switch (fileType) {
        case DAbstractFileInfo::Videos:
            mediaType = DFMMediaInfo::Video;
            break;
        case DAbstractFileInfo::Audios:
            mediaType = DFMMediaInfo::Audio;
            break;
        case DAbstractFileInfo::Images:
            mediaType = DFMMediaInfo::Image;
            break;
        default:

            break;
        }

        if (mediaType != DFMMediaInfo::Other) {
            const QString &filePath = info->filePath();
            DFMMediaInfo *mediaInfo = nullptr;
            // iphone 中读media文件很慢，因此特殊处理
            if (filePath.contains(IPHONE_STAGING) && filePath.startsWith(MOBILE_ROOT_PATH)) {
                mediaInfo = new DFMMediaInfo(filePath, nullptr);
                // startReadInfo 可能会很慢，因此延时1秒后，待后面的代码执行完后再执行
                QTimer::singleShot(1000, [mediaInfo]() {
                    mediaInfo->startReadInfo();
                    // 立即析构会导致读取media的任务无法完成，因此延时析构，5s是实验后比较稳定的值
                    QTimer::singleShot(5000, [mediaInfo]() {
                        mediaInfo->deleteLater();
                    });
                });
            } else {
                mediaInfo = new DFMMediaInfo(filePath, layoutWidget);
                mediaInfo->startReadInfo();
            }
            QPointer<DFMMediaInfo> ptrMediaInfo = mediaInfo;
            QObject::connect(fileSignalManager, &FileSignalManager::requestCloseMediaInfo, layout, [ptrMediaInfo, this](const QString path){
                if (m_url.path() == path && ptrMediaInfo) {
                    ptrMediaInfo->stopReadInfo();
                }
            }, Qt::DirectConnection);

            QObject::connect(mediaInfo, &DFMMediaInfo::Finished, layout, [ = ]() {
                int frame_height = q->height();
                QString duration = mediaInfo->Value("Duration", mediaType);
                if (duration.isEmpty()) {
                    duration = mediaInfo->Value("Duration", DFMMediaInfo::General);
                }
                // mkv duration may be float ?  like '666.000', toInt() will failed
                bool ok = false;
                int ms = duration.toInt(&ok);
                if (!ok)
                    ms = static_cast<int>(duration.toFloat(&ok));

                int row = 2;
                if (ok) {
                    QTime t(0, 0);
                    t = t.addMSecs(ms);
                    QLabel *pixelKeyLabel = new SectionKeyLabel(QObject::tr("Duration"));
                    QLabel *pixelLabel = new SectionValueLabel;
                    pixelLabel->setText(t.toString("hh:mm:ss"));
                    layout->insertRow(row++, pixelKeyLabel, pixelLabel);
                    frame_height += 30;
                }

                bool okw = false, okh = false;
                int width = mediaInfo->Value("Width", mediaType).toInt(&okw);
                int height = mediaInfo->Value("Height", mediaType).toInt(&okh);
                if (okw && okh) {
                    QString text = QString("%1x%2").arg(width).arg(height);
                    QLabel *pixelKeyLabel = new SectionKeyLabel(QObject::tr("Dimension"));
                    QLabel *pixelLabel = new SectionValueLabel;
                    pixelLabel->setText(text);
                    layout->insertRow(row++, pixelKeyLabel, pixelLabel);
                    frame_height += 30;
                }

                if (ok || (okw && okh)) {
                    q->setFixedHeight(frame_height);
                }
            });
        }
    }

    /*if (!info->isVirtualEntry())*/ {
        SectionKeyLabel *typeSectionLabel = new SectionKeyLabel(QObject::tr("Type"));
        layout->addRow(typeSectionLabel, typeLabel);
    }

    if (info->isSymLink()) {
        SectionKeyLabel *linkPathSectionLabel = new SectionKeyLabel(QObject::tr("Location"));

        LinkSectionValueLabel *linkPathLabel = new LinkSectionValueLabel(info->symlinkTargetPath());
        linkPathLabel->setToolTip(info->symlinkTargetPath());
        linkPathLabel->setLinkTargetUrl(info->redirectedFileUrl());
        linkPathLabel->setOpenExternalLinks(true);
        linkPathLabel->setWordWrap(false);
        QString t = linkPathLabel->fontMetrics().elidedText(info->symlinkTargetPath(), Qt::ElideMiddle, 150);
        linkPathLabel->setText(t);
        layout->addRow(linkPathSectionLabel, linkPathLabel);
    }

    if (!info->isVirtualEntry()) {
        SectionKeyLabel *TimeCreatedSectionLabel = new SectionKeyLabel(QObject::tr("Time accessed"));
        SectionValueLabel *timeCreatedLabel = nullptr;
        SectionKeyLabel *TimeModifiedSectionLabel = nullptr;
        SectionValueLabel *timeModifiedLabel = nullptr;
        if (VaultController::isRootDirectory(info->fileUrl().toLocalFile())) {
            TimeModifiedSectionLabel = new SectionKeyLabel(QObject::tr("Time locked"));
            //! 保险箱根目录创建、访问、修改时间的读取
            DFM_NAMESPACE::DFMSettings setting(VAULT_TIME_CONFIG_FILE);
            timeCreatedLabel = new SectionValueLabel(setting.value(QString("VaultTime"), QString("InterviewTime")).toString());
            if (setting.value(QString("VaultTime"), QString("LockTime")).toString().isEmpty())
                timeModifiedLabel = new SectionValueLabel(setting.value(QString("VaultTime"), QString("InterviewTime")).toString());
            else
                timeModifiedLabel = new SectionValueLabel(setting.value(QString("VaultTime"), QString("LockTime")).toString());
        }else {
            TimeModifiedSectionLabel = new SectionKeyLabel(QObject::tr("Time modified"));
            timeCreatedLabel = new SectionValueLabel(info->lastReadDisplayName());
            timeModifiedLabel = new SectionValueLabel(info->lastModifiedDisplayName());
        }
        DFMGlobal::setToolTip(TimeCreatedSectionLabel);
        DFMGlobal::setToolTip(TimeModifiedSectionLabel);

        layout->addRow(TimeCreatedSectionLabel, timeCreatedLabel);
        layout->addRow(TimeModifiedSectionLabel, timeModifiedLabel);
    }

    if (info->fileUrl().isTrashFile() && info->fileUrl() != DUrl(TRASH_ROOT)) {
        QString pathStr = static_cast<const TrashFileInfo *>(info.constData())->sourceFilePath();
        SectionValueLabel *sourcePathLabel = new SectionValueLabel(pathStr);
        QString elidedStr = sourcePathLabel->fontMetrics().elidedText(pathStr, Qt::ElideMiddle, 150);
        sourcePathLabel->setToolTip(pathStr);
        sourcePathLabel->setWordWrap(false);
        sourcePathLabel->setText(elidedStr);
        SectionKeyLabel *sourcePathSectionLabel = new SectionKeyLabel(QObject::tr("Source path"));
        layout->addRow(sourcePathSectionLabel, sourcePathLabel);
    }

    //layout->setContentsMargins(0, 0, 40, 0);
    //! lixiang change
    layoutWidget->update();
    //! lixiang change
}

DFMFileBasicInfoWidget::DFMFileBasicInfoWidget(QWidget *parent)
    : QFrame(parent)
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

bool DFMFileBasicInfoWidget::showMediaInfo()
{
    Q_D(DFMFileBasicInfoWidget);
    return d->m_showMediaInfo;
}

void DFMFileBasicInfoWidget::setShowMediaInfo(bool visible)
{
    Q_D(DFMFileBasicInfoWidget);
    d->m_showMediaInfo = visible;
}

bool DFMFileBasicInfoWidget::showSummary()
{
    Q_D(DFMFileBasicInfoWidget);
    return d->m_showSummaryOnly;
}

void DFMFileBasicInfoWidget::setShowSummary(bool enable)
{
    Q_D(DFMFileBasicInfoWidget);
    d->m_showSummaryOnly = enable;
}

void DFMFileBasicInfoWidget::updateSizeText(qint64 size, int filesCount, int directoryCount)
{
    Q_D(DFMFileBasicInfoWidget);

    if (d->m_folderSizeLabel)
        d->m_folderSizeLabel->setText(FileUtils::formatSize(size));
    if (d->m_containSizeLabel)
        d->m_containSizeLabel->setText(QString::number(filesCount + directoryCount));
}

void DFMFileBasicInfoWidget::showEvent(QShowEvent *event)
{
    Q_D(DFMFileBasicInfoWidget);
    if (d->m_folderSizeLabel && d->m_folderSizeLabel->text().isEmpty()) {
        d->startCalcFolderSize();
    }

    return QFrame::showEvent(event);
}

DFM_END_NAMESPACE
