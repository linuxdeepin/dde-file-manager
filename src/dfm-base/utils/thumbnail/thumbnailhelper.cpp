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

    //images
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
    if (!info || !info->isAttributes(FileInfo::FileIsType::kIsReadable) || !info->isAttributes(FileInfo::FileIsType::kIsFile))
        return false;

    qint64 fileSize = info->size();
    if (fileSize <= 0)
        return false;

    const QMimeType &mime = mimeDatabase.mimeTypeForFile(url);
    if (fileSize > sizeLimit(mime) && !mime.name().startsWith("video/"))
        return false;

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

    if (mimeName.startsWith("image"))
        return checkStatus(Application::kPreviewImage);

    if (mimeName.startsWith("audio")
        || MimeTypeDisplayManager::instance()->supportAudioMimeTypes().contains(mimeName))
        return checkStatus(Application::kPreviewAudio);

    if (mimeName.startsWith("video")
        || MimeTypeDisplayManager::instance()->supportVideoMimeTypes().contains(mimeName))
        return checkStatus(Application::kPreviewVideo);

    if (mimeName == Mime::kTypeTextPlain)
        return checkStatus(Application::kPreviewTextFile);

    if (Q_LIKELY(mimeList.contains(Mime::kTypeAppPdf)
                 || mimeName == Mime::kTypeAppCRRMedia
                 || mimeName == Mime::kTypeAppMxf))
        return checkStatus(Application::kPreviewDocumentFile);

    return false;
}

void ThumbnailHelper::makePath(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");
}

QString ThumbnailHelper::saveThumbnail(const QUrl &url, const QImage &img, ThumbnailSize size)
{
    if (img.isNull())
        return "";

    auto info = InfoFactory::create<FileInfo>(url);
    if (!info)
        return "";

    const QString &fileUrl = url.toString(QUrl::FullyEncoded);
    const QString &thumbnailName = ThumbnailHelper::dataToMd5Hex(fileUrl.toLocal8Bit()) + kFormat;
    const QString &thumbnailPath = ThumbnailHelper::sizeToFilePath(size);
    const QString &thumbnailFilePath = DFMIO::DFMUtils::buildFilePath(thumbnailPath.toStdString().c_str(), thumbnailName.toStdString().c_str(), nullptr);
    const qint64 fileModify = info->timeOf(TimeInfoType::kLastModifiedSecond).toLongLong();

    makePath(thumbnailPath);

    QMetaObject::invokeMethod(QCoreApplication::instance(), [img, thumbnailFilePath, fileUrl, fileModify]() {
        Q_ASSERT(QThread::currentThread() == qApp->thread());
        QImage tmpImg = img;
        tmpImg.setText(QT_STRINGIFY(Thumb::URL), fileUrl);
        tmpImg.setText(QT_STRINGIFY(Thumb::MTime), QString::number(fileModify));
        if (!tmpImg.save(thumbnailFilePath, Q_NULLPTR, 50)) {
            qCWarning(logDFMBase) << "thumbnail: save failed." << fileUrl;
        }
    },
                              Qt::QueuedConnection);

    return thumbnailFilePath;
}

QImage ThumbnailHelper::thumbnailImage(const QUrl &fileUrl, ThumbnailSize size)
{
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(fileUrl);
    if (!fileInfo)
        return {};

    const QString &dirPath = fileInfo->pathOf(PathInfoType::kPath);
    const QString &filePath = fileInfo->pathOf(PathInfoType::kFilePath);
    if (dirPath.isEmpty() || filePath.isEmpty())
        return {};

    if (defaultThumbnailDirs().contains(dirPath)) {
        QImage img(filePath);
        img.setText(QT_STRINGIFY(Thumb::Path), filePath);
        return img;
    }

    const QString thumbnailName = dataToMd5Hex((QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded)).toLocal8Bit()) + kFormat;
    QString thumbnail = DFMIO::DFMUtils::buildFilePath(sizeToFilePath(size).toStdString().c_str(), thumbnailName.toStdString().c_str(), nullptr);
    if (!DFMIO::DFile(thumbnail).exists())
        return {};

    QImageReader ir(thumbnail, QByteArray(kFormat).mid(1));
    if (!ir.canRead()) {
        LocalFileHandler().deleteFileRecursive(QUrl::fromLocalFile(thumbnail));
        return {};
    }
    ir.setAutoDetectImageFormat(false);

    QImage image = ir.read();
    const qint64 fileModify = fileInfo->timeOf(TimeInfoType::kLastModifiedSecond).toLongLong();
    if (!image.isNull() && image.text(QT_STRINGIFY(Thumb::MTime)).toInt() != static_cast<int>(fileModify)) {
        LocalFileHandler().deleteFileRecursive(QUrl::fromLocalFile(thumbnail));
        return {};
    }

    image.setText(QT_STRINGIFY(Thumb::Path), thumbnail);
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
        if (!info || !info->exists())
            return false;

        fileUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kAbsoluteFilePath));
        if (!fileUrl.isLocalFile())
            return false;
    }

    bool enable { true };
    if (FileUtils::isMtpFile(fileUrl)) {   // 是否是mtpfile
        enable = DConfigManager::instance()->value("org.deepin.dde.file-manager.preview", "mtpThumbnailEnable", true).toBool();
    } else if (DevProxyMng->isFileOfProtocolMounts(fileUrl.path())) {   // 是否是协议设备
        enable = Application::instance()->genericAttribute(Application::kShowThunmbnailInRemote).toBool();
    }

    if (!enable)
        return false;

    const QMimeType &mime = mimeDatabase.mimeTypeForFile(fileUrl);
    return checkMimeTypeSupport(mime);
}
