// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basicwidget.h"
#include "events/propertyeventcall.h"
#include "utils/propertydialogmanager.h"
#include "utils/mediainfofetchworker.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfileinfo.h>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif
#include <DGuiApplicationHelper>

#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
#include <QSet>
#include <QDBusInterface>
#include <QImageReader>

static constexpr int kSpacingHeight { 2 };
static constexpr int kLeftContentsMargins { 10 };
static constexpr int kRightContentsMargins { 10 };
static constexpr int kFrameWidth { 360 };
static constexpr int kItemWidth { 340 };
static constexpr int kLeftWidgetWidth { 70 };
static constexpr int kRightWidgetWidth { 255 };

Q_DECLARE_METATYPE(QList<QUrl> *)

USING_IO_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

BasicWidget::BasicWidget(QWidget *parent)
    : DArrowLineDrawer(parent),
      infoFetchWorker(new MediaInfoFetchWorker)

{
    initUI();
    fileCalculationUtils = new FileStatisticsJob;
    fileCalculationUtils->setFileHints(FileStatisticsJob::FileHint::kNoFollowSymlink);

    connect(&fetchThread, &QThread::finished, infoFetchWorker, &QObject::deleteLater);
    infoFetchWorker->moveToThread(&fetchThread);
    fetchThread.start();
}

BasicWidget::~BasicWidget()
{
    fileCalculationUtils->deleteLater();
    if (fetchThread.isRunning()) {
        fetchThread.quit();
        fetchThread.wait(5000);
    }
}

int BasicWidget::expansionPreditHeight()
{
    int itemCount = hideCheckBox ? 0 : 1;
    int allItemHeight { 0 };
    QMultiMap<BasicFieldExpandEnum, DFMBASE_NAMESPACE::KeyValueLabel *>::const_iterator itr = fieldMap.begin();
    for (; itr != fieldMap.end(); ++itr) {
        if (itr.value() && itr.value()->isVisible()) {
            allItemHeight += itr.value()->height();
            ++itemCount;
        }
    }
    if (hideFile)
        allItemHeight += (hideCheckBox ? 0 : hideFile->height());

    int allSpaceHeight = (itemCount - 1) * kSpacingHeight;
    return allSpaceHeight + allItemHeight;
}

void BasicWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    setTitle(QString(tr("Basic info")));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType::T6, QFont::DemiBold);

    setExpand(true);

    frameMain = new QFrame(this);
    frameMain->setFixedWidth(kFrameWidth);

    fileSize = createValueLabel(frameMain, tr("Size"));
    fileCount = createValueLabel(frameMain, tr("Contains"));
    fileType = createValueLabel(frameMain, tr("Type"));
    fileLocation = createValueLabel(frameMain, tr("Location"));
    fileCreated = createValueLabel(frameMain, tr("Created"));
    fileAccessed = createValueLabel(frameMain, tr("Accessed"));
    fileModified = createValueLabel(frameMain, tr("Modified"));

    hideFile = new DCheckBox(frameMain);
    DFontSizeManager::instance()->bind(hideFile, DFontSizeManager::SizeType::T7, QFont::Normal);
    hideFile->setText(tr("Hide this file"));
    hideFile->setToolTip(hideFile->text());

    fileMediaResolution = createValueLabel(frameMain, tr("Resolution"));
    fileMediaDuration = createValueLabel(frameMain, tr("Duration"));
}

KeyValueLabel *BasicWidget::createValueLabel(QFrame *frame, QString leftValue)
{
    KeyValueLabel *res = new KeyValueLabel(frame);
    res->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::Medium);
    res->setLeftValue(leftValue, Qt::ElideMiddle, Qt::AlignLeft | Qt::AlignVCenter, true);
    res->setRightFontSizeWeight(DFontSizeManager::SizeType::T8, QFont::Light);
    res->leftWidget()->setMinimumWidth(kLeftWidgetWidth);
    res->rightWidget()->setMinimumWidth(kRightWidgetWidth);
    return res;
}

void BasicWidget::basicExpand(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> fieldCondition = PropertyDialogManager::instance().createBasicViewExtensionField(url);

    QList<BasicExpandType> keys = fieldCondition.keys();
    for (BasicExpandType key : keys) {
        BasicExpandMap expand = fieldCondition.value(key);
        QList<BasicFieldExpandEnum> filterEnumList = expand.keys();
        switch (key) {
        case kFieldInsert: {
            for (BasicFieldExpandEnum k : filterEnumList) {
                QList<QPair<QString, QString>> fieldlist = expand.values(k);
                for (QPair<QString, QString> field : fieldlist) {
                    KeyValueLabel *expandLabel = createValueLabel(this, field.first);
                    expandLabel->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignVCenter, true);
                    fieldMap.insert(k, expandLabel);
                }
            }
        } break;
        case kFieldReplace: {
            for (BasicFieldExpandEnum k : filterEnumList) {
                QPair<QString, QString> field = expand.value(k);
                fieldMap.value(k)->setLeftValue(field.first, Qt::ElideMiddle, Qt::AlignLeft | Qt::AlignVCenter, true);
                fieldMap.value(k)->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignVCenter, true);
            }
        } break;
        }
    }

    DLabel *label = new DLabel(frameMain);
#ifdef DTKWIDGET_CLASS_DSizeMode
    label->setFixedWidth(DSizeModeHelper::element(90, 90));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [label, this]() {
        label->setFixedWidth(DSizeModeHelper::element(90, 90));
    });
#else
    label->setFixedWidth(80);
#endif
    QHBoxLayout *gl = new QHBoxLayout;
    gl->setContentsMargins(0, 0, 0, 0);
    gl->addWidget(label);
    gl->addWidget(hideFile, Qt::AlignLeft);

    QFrame *tempFrame = new QFrame(frameMain);
    tempFrame->setLayout(gl);
    tempFrame->setFixedWidth(kItemWidth);

    layoutMain = new QGridLayout;
    layoutMain->setContentsMargins(kLeftContentsMargins, 0, 0, kRightContentsMargins);
    layoutMain->setSpacing(kSpacingHeight);
    int row = 0;
    QList<BasicFieldExpandEnum> fields = fieldMap.keys();
    std::sort(fields.begin(), fields.end());
    fields.erase(std::unique(fields.begin(), fields.end()), fields.end());
    for (BasicFieldExpandEnum &key : fields) {
        QList<KeyValueLabel *> kvls = fieldMap.values(key);
        for (int i = kvls.count() - 1; i >= 0; --i) {
            KeyValueLabel *kvl = kvls[i];
            layoutMain->addWidget(kvl, row, 0, 1, 6);
            row++;
        }
    }
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QStringList list = url.path().split("/", QString::SkipEmptyParts);
#else
    QStringList &&list = url.path().split("/", Qt::SkipEmptyParts);
#endif
    if (!list.isEmpty() && url.isValid() && list.last().startsWith(".")) {
        tempFrame->hide();
        hideCheckBox = true;
    } else {
        layoutMain->addWidget(tempFrame, row, 0, 1, 6);
        hideCheckBox = false;
    }
    layoutMain->setColumnStretch(0, 1);

    frameMain->setLayout(layoutMain);
    setContent(frameMain);
}

void BasicWidget::basicFieldFilter(const QUrl &url)
{
    PropertyFilterType fieldFilter = PropertyDialogManager::instance().basicFiledFiltes(url);
    if (fieldFilter & PropertyFilterType::kFileSizeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileSize);
        fileSize->deleteLater();
        fileSize = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileTypeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileType);
        fileType->deleteLater();
        fileType = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileCountFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileCount);
        fileCount->deleteLater();
        fileCount = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFilePositionFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFilePosition);
        fileLocation->deleteLater();
        fileLocation = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileCreateTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileCreateTime);
        fileCreated->deleteLater();
        fileCreated = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileAccessedTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileAccessedTime);
        fileAccessed->deleteLater();
        fileAccessed = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileModifiedTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileModifiedTime);
        fileModified->deleteLater();
        fileModified = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileMediaResolutionFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileMediaResolution);
        fileMediaResolution->deleteLater();
        fileMediaResolution = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileMediaDurationFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileMediaDuration);
        fileMediaDuration->deleteLater();
        fileMediaDuration = nullptr;
    }
}

void BasicWidget::basicFill(const QUrl &url)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (info.isNull())
        return;
    if (!info->canAttributes(CanableInfoType::kCanHidden))
        hideFile->setEnabled(false);

    if (info->isAttributes(OptInfoType::kIsHidden))
        hideFile->setChecked(true);

    connect(hideFile, &DCheckBox::checkStateChanged, this, &BasicWidget::slotFileHide);

    if (fileLocation && fileLocation->RightValue().isEmpty()) {
        fileLocation->setRightValue(info->isAttributes(OptInfoType::kIsSymLink) ? info->pathOf(PathInfoType::kSymLinkTarget)
                                                                                : info->pathOf(PathInfoType::kAbsoluteFilePath),
                                    Qt::ElideMiddle, Qt::AlignVCenter, true);
        connect(fileLocation, &KeyValueLabel::valueAreaClicked, this, &BasicWidget::slotOpenFileLocation);
    }

    if (fileCreated && fileCreated->RightValue().isEmpty()) {
        auto birthTime = info->timeOf(TimeInfoType::kBirthTime).value<QDateTime>();
        birthTime.isValid() ? fileCreated->setRightValue(birthTime.toString(FileUtils::dateTimeFormat()), Qt::ElideNone, Qt::AlignVCenter, true)
                            : fileCreated->setVisible(false);
    }
    if (fileAccessed && fileAccessed->RightValue().isEmpty()) {
        auto lastRead = info->timeOf(TimeInfoType::kLastRead).value<QDateTime>();
        lastRead.isValid() ? fileAccessed->setRightValue(lastRead.toString(FileUtils::dateTimeFormat()), Qt::ElideNone, Qt::AlignVCenter, true)
                           : fileAccessed->setVisible(false);
    }
    if (fileModified && fileModified->RightValue().isEmpty()) {
        auto lastModified = info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
        lastModified.isValid() ? fileModified->setRightValue(lastModified.toString(FileUtils::dateTimeFormat()), Qt::ElideNone, Qt::AlignVCenter, true)
                               : fileModified->setVisible(false);
    }
    if (fileSize && fileSize->RightValue().isEmpty()) {
        fSize = info->size();
        fCount = 1;
        fileSize->setRightValue(FileUtils::formatSize(fSize), Qt::ElideNone, Qt::AlignVCenter, true);
    }

    if (fileMediaResolution && fileMediaResolution->RightValue().isEmpty())
        fileMediaResolution->setVisible(false);
    if (fileMediaDuration && fileMediaDuration->RightValue().isEmpty())
        fileMediaDuration->setVisible(false);

    if (fileType && fileType->RightValue().isEmpty()) {
        FileInfo::FileType type = info->fileType();
        fileType->setRightValue(info->displayOf(DisPlayInfoType::kMimeTypeDisplayName), Qt::ElideMiddle, Qt::AlignVCenter, true);
        if (type == FileInfo::FileType::kDirectory && fileCount && fileCount->RightValue().isEmpty()) {
            fileCount->setRightValue(tr("%1 item").arg(0), Qt::ElideNone, Qt::AlignVCenter, true);
            connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &BasicWidget::slotFileCountAndSizeChange);
            if (info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)) {
                fileCalculationUtils->start(QList<QUrl>() << info->urlOf(UrlInfoType::kRedirectedFileUrl));
            } else {
                fileCalculationUtils->start(QList<QUrl>() << url);
            }
        } else {
            layoutMain->removeWidget(fileCount);
            fieldMap.remove(BasicFieldExpandEnum::kFileCount);
            delete fileCount;
            fileCount = nullptr;
        }

        QUrl localUrl = url;
        QList<QUrl> urls {};
        bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
        if (ok && !urls.isEmpty())
            localUrl = urls.first();
        FileInfoPointer localinfo = InfoFactory::create<FileInfo>(localUrl);
        const QString &mimeName { localinfo->nameOf(NameInfoType::kMimeTypeName) };
        type = MimeTypeDisplayManager::instance()->displayNameToEnum(mimeName);
        QList<DFileInfo::AttributeExtendID> extenList;
        if (type == FileInfo::FileType::kVideos) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &BasicWidget::videoExtenInfo);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kVideo, extenList);
            if (!mediaAttributes.isEmpty())
                videoExtenInfo(url, mediaAttributes);
            fileMediaResolution->setVisible(true);
            fileMediaDuration->setVisible(true);
        } else if (type == FileInfo::FileType::kImages) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &BasicWidget::imageExtenInfo);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kImage, extenList);
            if (!mediaAttributes.isEmpty())
                imageExtenInfo(url, mediaAttributes);
            fileMediaResolution->setVisible(true);
        } else if (type == FileInfo::FileType::kAudios) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &BasicWidget::audioExtenInfo);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kAudio, extenList);
            if (!mediaAttributes.isEmpty())
                audioExtenInfo(url, mediaAttributes);
            fileMediaDuration->setVisible(true);
        }
    }
}

void BasicWidget::initFileMap()
{
    fieldMap.insert(BasicFieldExpandEnum::kFileSize, fileSize);
    fieldMap.insert(BasicFieldExpandEnum::kFileCount, fileCount);
    fieldMap.insert(BasicFieldExpandEnum::kFileType, fileType);
    fieldMap.insert(BasicFieldExpandEnum::kFilePosition, fileLocation);
    fieldMap.insert(BasicFieldExpandEnum::kFileCreateTime, fileCreated);
    fieldMap.insert(BasicFieldExpandEnum::kFileAccessedTime, fileAccessed);
    fieldMap.insert(BasicFieldExpandEnum::kFileModifiedTime, fileModified);
    fieldMap.insert(BasicFieldExpandEnum::kFileMediaResolution, fileMediaResolution);
    fieldMap.insert(BasicFieldExpandEnum::kFileMediaDuration, fileMediaDuration);
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

void BasicWidget::updateFileUrl(const QUrl &url)
{
    currentUrl = url;
}

void BasicWidget::slotFileCountAndSizeChange(qint64 size, int filesCount, int directoryCount)
{
    fSize = size;
    fileSize->setRightValue(FileUtils::formatSize(size), Qt::ElideNone, Qt::AlignVCenter, true);

    fCount = filesCount + (directoryCount > 1 ? directoryCount - 1 : 0);
    QString txt = fCount > 1 ? tr("%1 items") : tr("%1 item");
    fileCount->setRightValue(txt.arg(fCount), Qt::ElideNone, Qt::AlignVCenter, true);
}

void BasicWidget::slotFileHide(Qt::CheckState state)
{
    Q_UNUSED(state)
    auto winID = qApp->activeWindow() ? qApp->activeWindow()->winId() : 0;
    PropertyEventCall::sendFileHide(winID, { currentUrl });
}

void BasicWidget::slotOpenFileLocation()
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(currentUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (info.isNull())
        return;

    // Determine the target path: for symlinks, use the link target; for others, use the file itself
    QString targetPath = info->isAttributes(OptInfoType::kIsSymLink)
            ? info->pathOf(PathInfoType::kSymLinkTarget)
            : info->pathOf(PathInfoType::kAbsoluteFilePath);

    const QUrl targetUrl = QUrl::fromLocalFile(targetPath);
    const auto &targetInfo = InfoFactory::create<FileInfo>(targetUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (targetInfo.isNull())
        return;

    QUrl parentUrl = targetInfo->urlOf(UrlInfoType::kParentUrl);
    parentUrl.setQuery("selectUrl=" + targetUrl.toString());

    // Try to use DBus interface first
    QDBusInterface interface("org.freedesktop.FileManager1",
                             "/org/freedesktop/FileManager1",
                             "org.freedesktop.FileManager1",
                             QDBusConnection::sessionBus());
    interface.setTimeout(1000);

    if (interface.isValid()) {
        fmInfo() << "Opening file location via DBus:" << targetUrl.toString();
        interface.call("ShowItems", QStringList() << targetUrl.toString(), "dfmplugin-propertydialog");
    } else {
        fmWarning() << "DBus org.freedesktop.FileManager1 not available, using fallback";
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, parentUrl);
    }
}

void BasicWidget::closeEvent(QCloseEvent *event)
{
    DArrowLineDrawer::closeEvent(event);
}

void BasicWidget::imageExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url != currentUrl || properties.isEmpty()) {
        fileMediaResolution->setVisible(false);
        return;
    }

    // Try to get dimensions from properties
    int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
    int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();

    if (width == 0 || height == 0) {
        fileMediaResolution->setVisible(false);
        return;
    }

    const QString &imgSizeStr = QString::number(width) + "x" + QString::number(height);
    fileMediaResolution->setRightValue(imgSizeStr, Qt::ElideNone, Qt::AlignVCenter, true);
}

void BasicWidget::videoExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url != currentUrl || properties.isEmpty()) {
        fileMediaResolution->setVisible(false);
        fileMediaDuration->setVisible(false);
        return;
    }

    int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
    int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();
    const QString &videoResolutionStr = QString::number(width) + "x" + QString::number(height);
    fileMediaResolution->setRightValue(videoResolutionStr, Qt::ElideNone, Qt::AlignVCenter, true);

    int duration = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration].toInt();
    if (duration != 0) {
        QTime t(0, 0, 0);
        t = t.addMSecs(duration);
        const QString &durationStr = t.toString("hh:mm:ss");
        fileMediaDuration->setRightValue(durationStr, Qt::ElideNone, Qt::AlignVCenter, true);
    } else {
        QString localFile = url.toLocalFile();
        connect(infoFetchWorker, &MediaInfoFetchWorker::durationReady,
                this, [this](const QString &duration) {
                    if (!duration.isEmpty()) {
                        fileMediaDuration->setRightValue(duration);
                    } else {
                        fileMediaDuration->setVisible(false);
                    }
                });

        QMetaObject::invokeMethod(infoFetchWorker, "getDuration",
                                  Qt::QueuedConnection, Q_ARG(QString, localFile));
    }
}

void BasicWidget::audioExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if (url != currentUrl || properties.isEmpty()) {
        fileMediaResolution->setVisible(false);
        fileMediaDuration->setVisible(false);
        return;
    }

    int duration = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration].toInt();
    if (duration != 0) {
        QTime t(0, 0, 0);
        t = t.addMSecs(duration);
        const QString &durationStr = t.toString("hh:mm:ss");
        fileMediaDuration->setRightValue(durationStr, Qt::ElideNone, Qt::AlignVCenter, true);
    } else {
        QString localFile = url.toLocalFile();
        connect(infoFetchWorker, &MediaInfoFetchWorker::durationReady,
                this, [this](const QString &duration) {
                    if (!duration.isEmpty()) {
                        fileMediaDuration->setRightValue(duration);
                    } else {
                        fileMediaDuration->setVisible(false);
                    }
                });

        QMetaObject::invokeMethod(infoFetchWorker, "getDuration",
                                  Qt::QueuedConnection, Q_ARG(QString, localFile));
    }
}
