// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreviewworker.h"
#include "imagepreviewwidget.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/dconfig/global_dconf_defines.h>

#include <dfm-framework/dpf.h>

#include <QImageReader>
#include <QMovie>
#include <QApplication>

Q_DECLARE_METATYPE(QString *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_detailspace;

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE) };

ImagePreviewWorker::ImagePreviewWorker(QObject *parent)
    : QObject(parent)
{
}

ImagePreviewWorker::~ImagePreviewWorker()
{
}

void ImagePreviewWorker::stop()
{
    m_stopped.storeRelaxed(true);
}

void ImagePreviewWorker::loadPreview(const QUrl &url, const QSize &targetSize)
{
    if (m_stopped.loadRelaxed()) {
        return;
    }

    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (!info) {
        Q_EMIT needIconFallback(url, targetSize);
        return;
    }

    const QString mimeType = info->nameOf(NameInfoType::kMimeTypeName);
    const QString filePath = info->pathOf(PathInfoType::kAbsoluteFilePath);
    const qint64 fileSize = info->size();

    QPixmap result;

    // Strategy 1: For animated image types (GIF, etc.), verify with QMovie before emitting signal
    if (!mimeType.isEmpty() && ImagePreviewWidget::isAnimatedMimeType(mimeType)) {
        // Verify the file is actually a valid animated image using QMovie
        QMovie movie(filePath);
        if (movie.isValid() && movie.frameCount() > 1) {
            // Valid animated image - delegate to QMovie in main thread
            Q_EMIT animatedImageReady(url, filePath);
            return;
        }
        // Invalid or single-frame - fall through to static image loading
    }

    // Strategy 2: For static image types, load original image (with remote/optical size check)
    if (isImageMimeType(mimeType)) {
        // Check if we should skip original image loading for remote/optical files
        if (!shouldSkipOriginalImageLoad(url, fileSize)) {
            result = loadOriginalImage(filePath, targetSize);
            if (!result.isNull()) {
                Q_EMIT previewReady(url, result);
                return;
            }
        }
        // If skipped or loading failed, fall through to thumbnail
    }

    // Strategy 3: Try thumbnail
    result = loadThumbnail(url, targetSize);
    if (!result.isNull()) {
        Q_EMIT previewReady(url, result);
        return;
    }

    // Strategy 4: Request hook/icon from main thread (must be called on main thread)
    Q_EMIT needIconFallback(url, targetSize);
}

bool ImagePreviewWorker::isImageMimeType(const QString &mimeType) const
{
    return mimeType.startsWith("image/");
}

bool ImagePreviewWorker::shouldSkipOriginalImageLoad(const QUrl &url, qint64 fileSize) const
{
    // Check if file is on remote mount or optical device
    const QString filePath = url.isLocalFile() ? url.toLocalFile() : url.path();
    const bool isRemoteOrOptical = DevProxyMng->isFileOfProtocolMounts(filePath)
            || DevProxyMng->isFileFromOptical(filePath);

    if (!isRemoteOrOptical) {
        return false;
    }

    // Get configured maximum file size from DConfig
    using namespace GlobalDConfDefines;
    const qint64 maxSize = DConfigManager::instance()->value(
                                                             ConfigPath::kViewDConfName,
                                                             BaseConfig::kDetailViewRemoteImageMaxSize,
                                                             30 * 1024 * 1024   // Default: 30MB
                                                             )
                                   .toLongLong();

    // Skip original image loading if file size exceeds the configured limit
    const bool shouldSkip = fileSize > maxSize;
    if (shouldSkip) {
        fmInfo() << "detailview: skipping original image load for remote/optical file"
                 << url << "size:" << fileSize << "limit:" << maxSize;
    }

    return shouldSkip;
}

QPixmap ImagePreviewWorker::loadOriginalImage(const QString &filePath, const QSize &targetSize)
{
    QImageReader reader(filePath);
    reader.setAutoTransform(true);

    QSize originalSize = reader.size();
    if (!originalSize.isValid()) {
        return QPixmap();
    }

    qreal dpr = qApp->devicePixelRatio();
    QSize maxSize = targetSize * dpr;

    if (originalSize.width() > maxSize.width()
        || originalSize.height() > maxSize.height()) {
        QSize scaledSize = originalSize.scaled(maxSize, Qt::KeepAspectRatio);
        reader.setScaledSize(scaledSize);
    }

    QImage image = reader.read();
    if (image.isNull()) {
        return QPixmap();
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    pixmap.setDevicePixelRatio(dpr);
    return pixmap;
}

QPixmap ImagePreviewWorker::loadThumbnail(const QUrl &url, const QSize &targetSize)
{
    ThumbnailHelper helper;
    if (!helper.checkThumbEnable(url)) {
        return QPixmap();
    }

    QImage img = helper.thumbnailImage(url, Global::kLarge);
    if (img.isNull()) {
        return QPixmap();
    }

    qreal dpr = qApp->devicePixelRatio();
    QPixmap pixmap = QPixmap::fromImage(img);
    pixmap.setDevicePixelRatio(dpr);
    return pixmap;
}

// ========== ImagePreviewController ==========

ImagePreviewController::ImagePreviewController(QObject *parent)
    : QObject(parent)
{
    m_worker = new ImagePreviewWorker();
    m_worker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &ImagePreviewController::doLoadPreview,
            m_worker, &ImagePreviewWorker::loadPreview);
    connect(m_worker, &ImagePreviewWorker::previewReady,
            this, &ImagePreviewController::previewReady);
    connect(m_worker, &ImagePreviewWorker::animatedImageReady,
            this, &ImagePreviewController::onAnimatedImageReady);
    connect(m_worker, &ImagePreviewWorker::loadFailed,
            this, &ImagePreviewController::loadFailed);
    connect(m_worker, &ImagePreviewWorker::needIconFallback,
            this, &ImagePreviewController::onNeedIconFallback);

    // Connect to ThumbnailFactory's produceFinished signal for dynamic thumbnail updates
    connect(ThumbnailFactory::instance(), &ThumbnailFactory::produceFinished,
            this, &ImagePreviewController::onThumbnailProduced);

    m_workerThread.start();
}

ImagePreviewController::~ImagePreviewController()
{
    m_worker->stop();
    m_workerThread.quit();
    m_workerThread.wait(5000);
}

void ImagePreviewController::requestPreview(const QUrl &url, const QSize &targetSize)
{
    // Store current request parameters for potential thumbnail update
    m_currentUrl = url;
    m_currentTargetSize = targetSize;
    m_isWaitingForThumbnail = false;

    Q_EMIT doLoadPreview(url, targetSize);
}

void ImagePreviewController::onNeedIconFallback(const QUrl &url, const QSize &targetSize)
{
    // This runs on main thread, safe to call hooks and fileIcon()
    QPixmap result;
    qreal dpr = qApp->devicePixelRatio();

    // For icons, request a square size based on the larger dimension of targetSize
    // This prevents distortion - the icon will maintain its aspect ratio
    // and ImagePreviewWidget::paintEvent() will scale it to fit the preview area
    int iconSize = qMax(targetSize.width(), targetSize.height());
    QSize iconSizeSquare(iconSize, iconSize);

    // Strategy 1: Get custom icon from hook (highest priority in fallback)
    QString iconName;
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_Icon_Fetch", url, &iconName)
        && !iconName.isEmpty()) {
        // Request square icon with devicePixelRatio, QIcon will maintain its aspect ratio
        result = QIcon::fromTheme(iconName).pixmap(iconSizeSquare, dpr);
        if (!result.isNull()) {
            Q_EMIT previewReady(url, result);
            return;
        }
    }

    // Strategy 2: Fall back to file icon
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info) {
        QIcon icon = info->fileIcon();
        // Request square icon with devicePixelRatio, QIcon will maintain its aspect ratio
        result = icon.pixmap(iconSizeSquare, dpr);
        if (!result.isNull()) {
            Q_EMIT previewReady(url, result);
            // Mark that we're using icon fallback and waiting for potential thumbnail
            if (url == m_currentUrl) {
                m_isWaitingForThumbnail = true;
            }
            return;
        }
    }

    // Strategy 3: Final fallback - use "unknown" icon to ensure preview always shows something
    result = QIcon::fromTheme("unknown").pixmap(iconSizeSquare, dpr);
    if (!result.isNull()) {
        Q_EMIT previewReady(url, result);
        return;
    }

    // Should never reach here, but emit loadFailed as ultimate safety net
    fmWarning() << "detailview: failed to load any icon for" << url;
    Q_EMIT loadFailed(url);
}

void ImagePreviewController::onAnimatedImageReady(const QUrl &url, const QString &filePath)
{
    // Forward animated image signal
    Q_EMIT animatedImageReady(url, filePath);
}

void ImagePreviewController::onThumbnailProduced(const QUrl &url, const QString &thumbnailPath)
{
    Q_UNUSED(thumbnailPath)

    // Only reload if:
    // 1. This thumbnail belongs to the currently displayed file
    // 2. We are currently showing icon fallback (waiting for thumbnail)
    if (url == m_currentUrl && m_isWaitingForThumbnail) {
        qCDebug(logDFMBase) << "thumbnail: dynamically updating preview for:" << url;

        // Reset waiting flag to prevent duplicate reloads
        m_isWaitingForThumbnail = false;

        // Reload preview with current parameters
        // This will go through the full loading pipeline again, but now the thumbnail exists
        Q_EMIT doLoadPreview(m_currentUrl, m_currentTargetSize);
    }
}
