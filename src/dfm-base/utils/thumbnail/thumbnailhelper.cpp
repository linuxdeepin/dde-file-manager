// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailhelper.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QImageReader>
#include <QDir>

#include <sys/stat.h>

#include <sys/stat.h>

static constexpr qint64 kDefaultSizeLimit = 1024 * 1024 * 20;   // 20MB
static constexpr char kFormat[] { ".png" };

using namespace dfmbase;
DFMGLOBAL_USE_NAMESPACE

ThumbnailHelper::ThumbnailHelper()
{
}

void ThumbnailHelper::initSizeLimit()
{
    sizeLimitHash.reserve(28);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeTextPlain), 1024 * 1024);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeAppPdf), INT64_MAX);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeAppVRRMedia), INT64_MAX);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeAppVMAsf), INT64_MAX);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeAppMxf), INT64_MAX);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeAppPptx), INT64_MAX);

    // images
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImageIef), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImageTiff), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImageXTMultipage), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImageVDMultipage), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImageXADng), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImageJpeg), 1024 * 1024 * 30);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImagePng), 1024 * 1024 * 30);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeImagePipeg), 1024 * 1024 * 30);
    // High file limit size only for FLAC files.
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeAudioFlac), INT64_MAX);

    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::Mime::kTypeAppAppimage), INT64_MAX);

    qCInfo(logDFMBase) << "thumbnail: initialized size limits for" << sizeLimitHash.size() << "mime types";
}

const QStringList &ThumbnailHelper::defaultThumbnailDirs()
{
    const static QStringList dirs {
        StandardPaths::location(StandardPaths::StandardLocation::kThumbnailSmallPath),
        StandardPaths::location(StandardPaths::StandardLocation::kThumbnailNormalPath),
        StandardPaths::location(StandardPaths::StandardLocation::kThumbnailLargePath),
        StandardPaths::location(StandardPaths::kThumbnailFailPath)
    };
    return dirs;
}

bool ThumbnailHelper::canGenerateThumbnail(const QUrl &url)
{
    const auto &info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!info || !info->isAttributes(FileInfo::FileIsType::kIsReadable) || !info->isAttributes(FileInfo::FileIsType::kIsFile)) {
        qCDebug(logDFMBase) << "thumbnail: file is not readable or not a regular file:" << url;
        return false;
    }

    qint64 fileSize = info->size();
    if (fileSize <= 0) {
        qCDebug(logDFMBase) << "thumbnail: file size is invalid:" << fileSize << "for:" << url;
        return false;
    }

    const QMimeType &mime = mimeDatabase.mimeTypeForFile(url);
    qint64 limit = sizeLimit(mime);
    if (fileSize > limit && !mime.name().startsWith("video/")) {
        qCDebug(logDFMBase) << "thumbnail: file size" << fileSize << "exceeds limit" << limit << "for mime type:" << mime.name() << "file:" << url;
        return false;
    }

    return true;
}

bool ThumbnailHelper::checkMimeTypeSupport(const QMimeType &mime)
{
    const QString &mimeName = mime.name();
    QStringList mimeList = { mimeName };
    mimeList.append(mime.parentMimeTypes());

    auto checkStatus = [](Application::GenericAttribute attr) {
        return Application::instance()->genericAttribute(attr).toBool();
    };

    if (mimeName.startsWith("image")) {
        bool supported = checkStatus(Application::kPreviewImage);
        return supported;
    }

    if (mimeName.startsWith("audio")
        || MimeTypeDisplayManager::instance()->supportAudioMimeTypes().contains(mimeName)) {
        bool supported = checkStatus(Application::kPreviewAudio);
        return supported;
    }

    if (mimeName.startsWith("video")
        || MimeTypeDisplayManager::instance()->supportVideoMimeTypes().contains(mimeName)) {
        bool supported = checkStatus(Application::kPreviewVideo);
        return supported;
    }

    if (mimeName == Mime::kTypeTextPlain) {
        bool supported = checkStatus(Application::kPreviewTextFile);
        return supported;
    }

    if (Q_LIKELY(mimeList.contains(Mime::kTypeAppPdf)
                 || mimeName == Mime::kTypeAppCRRMedia
                 || mimeName == Mime::kTypeAppMxf)
        || mimeName == Mime::kTypeAppPptx) {
        bool supported = checkStatus(Application::kPreviewDocumentFile);
        return supported;
    }

    // appimage is executable package, should display icon
    if (mimeName == Mime::kTypeAppAppimage) {
        return true;
    }

    return false;
}

void ThumbnailHelper::makePath(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        bool success = dir.mkpath(".");
        if (success) {
            qCDebug(logDFMBase) << "thumbnail: created directory:" << path;
        } else {
            qCWarning(logDFMBase) << "thumbnail: failed to create directory:" << path;
        }
    }
}

QString ThumbnailHelper::saveThumbnail(const QUrl &url, const QImage &img, ThumbnailSize size)
{
    if (img.isNull()) {
        qCWarning(logDFMBase) << "thumbnail: cannot save null image for:" << url;
        return "";
    }

    auto info = InfoFactory::create<FileInfo>(url);
    if (!info) {
        qCWarning(logDFMBase) << "thumbnail: failed to create FileInfo for saving thumbnail:" << url;
        return "";
    }

    const QString &fileUrl = url.toString(QUrl::FullyEncoded);
    const QString &thumbnailName = ThumbnailHelper::dataToMd5Hex(fileUrl.toLocal8Bit()) + kFormat;
    const QString &thumbnailPath = ThumbnailHelper::sizeToFilePath(size);
    const QString &thumbnailFilePath = DFMIO::DFMUtils::buildFilePath(thumbnailPath.toStdString().c_str(), thumbnailName.toStdString().c_str(), nullptr);
    const qint64 fileModify = info->timeOf(TimeInfoType::kLastModifiedSecond).toLongLong();

    makePath(thumbnailPath);

    qCDebug(logDFMBase) << "thumbnail: saving thumbnail to:" << thumbnailFilePath << "for file:" << url;

    QMetaObject::invokeMethod(
            QCoreApplication::instance(), [img, thumbnailFilePath, fileUrl, fileModify]() {
                Q_ASSERT(QThread::currentThread() == qApp->thread());
                QImage tmpImg = img;
                tmpImg.setText(QT_STRINGIFY(Thumb::URL), fileUrl);
                tmpImg.setText(QT_STRINGIFY(Thumb::MTime), QString::number(fileModify));
                if (!tmpImg.save(thumbnailFilePath, Q_NULLPTR, 50)) {
                    qCWarning(logDFMBase) << "thumbnail: failed to save thumbnail file:" << thumbnailFilePath << "for:" << fileUrl;
                } else {
                    qCDebug(logDFMBase) << "thumbnail: successfully saved thumbnail:" << thumbnailFilePath;
                }
            },
            Qt::QueuedConnection);

    return thumbnailFilePath;
}

QImage ThumbnailHelper::thumbnailImage(const QUrl &fileUrl, ThumbnailSize size)
{
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(fileUrl);
    if (!fileInfo) {
        qCWarning(logDFMBase) << "thumbnail: failed to create FileInfo for loading thumbnail:" << fileUrl;
        return {};
    }

    const QString &dirPath = fileInfo->pathOf(PathInfoType::kPath);
    const QString &filePath = fileInfo->pathOf(PathInfoType::kFilePath);
    if (dirPath.isEmpty() || filePath.isEmpty()) {
        qCWarning(logDFMBase) << "thumbnail: invalid file paths for:" << fileUrl;
        return {};
    }

    if (defaultThumbnailDirs().contains(dirPath)) {
        QImage img(filePath);
        img.setText(QT_STRINGIFY(Thumb::Path), filePath);
        qCDebug(logDFMBase) << "thumbnail: loaded image from thumbnail directory:" << filePath;
        return img;
    }

    const QString thumbnailName = dataToMd5Hex((QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded)).toLocal8Bit()) + kFormat;
    QString thumbnail = DFMIO::DFMUtils::buildFilePath(sizeToFilePath(size).toStdString().c_str(), thumbnailName.toStdString().c_str(), nullptr);
    if (!DFMIO::DFile(thumbnail).exists()) {
        qCDebug(logDFMBase) << "thumbnail: cached thumbnail not found:" << thumbnail;
        return {};
    }

    QImageReader ir(thumbnail, QByteArray(kFormat).mid(1));
    if (!ir.canRead()) {
        qCWarning(logDFMBase) << "thumbnail: cannot read cached thumbnail, deleting:" << thumbnail;
        LocalFileHandler().deleteFileRecursive(QUrl::fromLocalFile(thumbnail));
        return {};
    }
    ir.setAutoDetectImageFormat(false);

    QImage image = ir.read();
    const qint64 fileModify = fileInfo->timeOf(TimeInfoType::kLastModifiedSecond).toLongLong();
    if (!image.isNull() && image.text(QT_STRINGIFY(Thumb::MTime)).toInt() != static_cast<int>(fileModify)) {
        qCDebug(logDFMBase) << "thumbnail: cached thumbnail is outdated, deleting:" << thumbnail;
        LocalFileHandler().deleteFileRecursive(QUrl::fromLocalFile(thumbnail));
        return {};
    }

    if (!image.isNull()) {
        image.setText(QT_STRINGIFY(Thumb::Path), thumbnail);
    }

    return image;
}

void ThumbnailHelper::setSizeLimit(const QMimeType &mime, qint64 size)
{
    if (mime.isValid() && !sizeLimitHash.contains(mime))
        sizeLimitHash.insert(mime, size);
}

qint64 ThumbnailHelper::sizeLimit(const QMimeType &mime)
{
    return sizeLimitHash.value(mime, kDefaultSizeLimit);
}

QString ThumbnailHelper::sizeToFilePath(ThumbnailSize size)
{
    switch (size) {
    case ThumbnailSize::kSmall:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailSmallPath);
    case ThumbnailSize::kNormal:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailNormalPath);
    case ThumbnailSize::kLarge:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailLargePath);
    }

    return "";
}

QByteArray ThumbnailHelper::dataToMd5Hex(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

bool ThumbnailHelper::checkThumbEnable(const QUrl &url)
{
    QUrl fileUrl { url };
    if (UrlRoute::isVirtual(fileUrl)) {
        auto info { InfoFactory::create<FileInfo>(fileUrl) };
        if (!info || !info->exists()) {
            qCDebug(logDFMBase) << "thumbnail: virtual file does not exist:" << fileUrl;
            return false;
        }

        fileUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kAbsoluteFilePath));
        if (!fileUrl.isLocalFile()) {
            qCDebug(logDFMBase) << "thumbnail: virtual file is not local:" << fileUrl;
            return false;
        }
    }

    bool enable { true };
    if (ProtocolUtils::isMTPFile(fileUrl)) {   // Check if it's MTP file
        enable = DConfigManager::instance()->value("org.deepin.dde.file-manager.preview", "mtpThumbnailEnable", true).toBool();
        qCDebug(logDFMBase) << "thumbnail: MTP file thumbnail enable status:" << enable << "for:" << fileUrl;
    } else if (DevProxyMng->isFileOfProtocolMounts(fileUrl.path())) {   // Check if it's protocol device
        enable = Application::instance()->genericAttribute(Application::kShowThunmbnailInRemote).toBool();
        qCDebug(logDFMBase) << "thumbnail: remote file thumbnail enable status:" << enable << "for:" << fileUrl;
    }

    if (!enable) {
        qCDebug(logDFMBase) << "thumbnail: thumbnail generation disabled for:" << fileUrl;
        return false;
    }

    const QMimeType &mime = mimeDatabase.mimeTypeForFile(fileUrl);
    bool supported = checkMimeTypeSupport(mime);
    return supported;
}
