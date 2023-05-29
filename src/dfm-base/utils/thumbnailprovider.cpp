// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailprovider.h"
#include "videothumbnailprovider.h"

#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-io/dfmio_utils.h>

#include <DThumbnailProvider>

#include <QCryptographicHash>
#include <QDateTime>
#include <QImageReader>
#include <QMimeType>
#include <QPainter>
#include <QProcess>
#include <QDebug>
#include <QtConcurrent>

// use original poppler api
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

constexpr char kFormat[] { ".png" };

inline QByteArray dataToMd5Hex(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

namespace dfmbase {

class ThumbnailProviderPrivate
{
public:
    explicit ThumbnailProviderPrivate(ThumbnailProvider *qq);

    void init();

    QString sizeToFilePath(ThumbnailProvider::Size size) const;
    uint64_t filePathToInode(QString filePath) const;

    ThumbnailProvider *q = nullptr;
    QString errorString;
    qint64 defaultSizeLimit = 1024 * 1024 * 20;
    QHash<QMimeType, qint64> sizeLimitHash;
    DMimeDatabase mimeDatabase;
    QLibrary *libMovieViewer = nullptr;
    QHash<QString, QString> keyToThumbnailTool;
    static QSet<QString> hasThumbnailMimeHash;
    QQueue<QUrl> produceAbsoluteFilePathQueue;
};

QSet<QString> ThumbnailProviderPrivate::hasThumbnailMimeHash;

ThumbnailProviderPrivate::ThumbnailProviderPrivate(ThumbnailProvider *qq)
    : q(qq)
{
}

void ThumbnailProviderPrivate::init()
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

QString ThumbnailProviderPrivate::sizeToFilePath(ThumbnailProvider::Size size) const
{
    switch (size) {
    case ThumbnailProvider::Size::kSmall:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailSmallPath);
    case ThumbnailProvider::Size::kNormal:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailNormalPath);
    case ThumbnailProvider::Size::kLarge:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailLargePath);
    }
    return "";
}

uint64_t ThumbnailProviderPrivate::filePathToInode(QString filePath) const
{
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    if (fileInfo.isNull())
        return static_cast<uint64_t>(-1);
    return fileInfo->extendAttributes(FileInfo::FileExtendedInfoType::kInode).toULongLong();
}

bool ThumbnailProvider::hasThumbnail(const QUrl &url) const
{
    const FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url);

    if (!fileInfo->isAttributes(OptInfoType::kIsReadable) || !fileInfo->isAttributes(OptInfoType::kIsFile))
        return false;

    qint64 fileSize = fileInfo->size();

    if (fileSize <= 0)
        return false;

    const QMimeType &mime = d->mimeDatabase.mimeTypeForFile(url);

    // todo lanxs 正在做拷贝的文件不去获取获取缩略图

    if (fileSize > sizeLimit(mime) && !mime.name().startsWith("video/"))
        return false;

    return hasThumbnail(mime);
}

bool ThumbnailProvider::hasThumbnail(const QMimeType &mimeType) const
{
    const QString &mime = mimeType.name();
    QStringList mimeTypeList = { mime };
    mimeTypeList.append(mimeType.parentMimeTypes());

    if (mime.startsWith("image") && !Application::instance()->genericAttribute(Application::kPreviewImage).toBool())
        return false;

    if ((mime.startsWith("audio") || DFMBASE_NAMESPACE::MimeTypeDisplayManager::instance()->supportAudioMimeTypes().contains(mime))
        && !Application::instance()->genericAttribute(Application::kPreviewAudio).toBool())
        return false;

    if ((mime.startsWith("video")
         || DFMBASE_NAMESPACE::MimeTypeDisplayManager::instance()->supportVideoMimeTypes().contains(mime))
        && !Application::instance()->genericAttribute(Application::kPreviewVideo).toBool())
        return false;

    if (mime == DFMGLOBAL_NAMESPACE::Mime::kTypeTextPlain && !Application::instance()->genericAttribute(Application::kPreviewTextFile).toBool())
        return false;

    if (Q_LIKELY(mimeTypeList.contains(DFMGLOBAL_NAMESPACE::Mime::kTypeAppPdf)
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppCRRMedia
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppMxf)
        && !Application::instance()->genericAttribute(Application::kPreviewDocumentFile).toBool()) {
        return false;
    }

    if (ThumbnailProviderPrivate::hasThumbnailMimeHash.contains(mime))
        return true;

    if (Q_LIKELY(mime.startsWith("image") || mime.startsWith("audio/") || mime.startsWith("video/"))) {
        ThumbnailProviderPrivate::hasThumbnailMimeHash.insert(mime);

        return true;
    }

    if (Q_LIKELY(mime == DFMGLOBAL_NAMESPACE::Mime::kTypeTextPlain
                 || mimeTypeList.contains(DFMGLOBAL_NAMESPACE::Mime::kTypeAppPdf)
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppVRRMedia
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppVMAsf
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppMxf)) {
        ThumbnailProviderPrivate::hasThumbnailMimeHash.insert(mime);

        return true;
    }

    if (DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->hasThumbnail(mimeType))
        return true;

    return false;
}

// true 1, false 0, invalid -1
int ThumbnailProvider::hasThumbnailFast(const QString &mime) const
{
    if (mime.startsWith("image") && !Application::instance()->genericAttribute(Application::kPreviewImage).toBool())
        return 0;

    if ((mime.startsWith("video")
         || DFMBASE_NAMESPACE::MimeTypeDisplayManager::instance()->supportVideoMimeTypes().contains(mime))
        && !Application::instance()->genericAttribute(Application::kPreviewVideo).toBool())
        return 0;

    if ((mime.startsWith("audio")
         || DFMBASE_NAMESPACE::MimeTypeDisplayManager::instance()->supportAudioMimeTypes().contains(mime))
        && !Application::instance()->genericAttribute(Application::kPreviewAudio).toBool())
        return 0;

    if (mime == DFMGLOBAL_NAMESPACE::Mime::kTypeTextPlain && !Application::instance()->genericAttribute(Application::kPreviewTextFile).toBool())
        return 0;

    if (Q_LIKELY(mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppCRRMedia
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppMxf || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppPdf)
        && !Application::instance()->genericAttribute(Application::kPreviewDocumentFile).toBool()) {
        return 0;
    }

    if (ThumbnailProviderPrivate::hasThumbnailMimeHash.contains(mime))
        return 1;

    if (Q_LIKELY(mime.startsWith("image") || mime.startsWith("audio/") || mime.startsWith("video/"))) {
        ThumbnailProviderPrivate::hasThumbnailMimeHash.insert(mime);

        return 1;
    }

    if (Q_LIKELY(mime == DFMGLOBAL_NAMESPACE::Mime::kTypeTextPlain
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppVRRMedia
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppVMAsf
                 || mime == DFMGLOBAL_NAMESPACE::Mime::kTypeAppMxf)) {
        ThumbnailProviderPrivate::hasThumbnailMimeHash.insert(mime);

        return 1;
    }

    return -1;
}

bool ThumbnailProvider::thumbnailEnable(const QUrl &url) const
{
    if (FileUtils::isMtpFile(url)) {
        return DConfigManager::instance()->value("org.deepin.dde.file-manager.preview", "mtpThumbnailEnable", true).toBool();
    } else if (FileUtils::isGvfsFile(url)) {
        return Application::instance()->genericAttribute(Application::kShowThunmbnailInRemote).toBool();
    } else if (DevProxyMng->isFileOfExternalBlockMounts(url.path())) {
        return true;
    }

    return false;
}

QPixmap ThumbnailProvider::thumbnailPixmap(const QUrl &fileUrl, Size size) const
{
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(fileUrl);
    if (!fileInfo)
        return QString();

    const QString &dirPath = fileInfo->pathOf(PathInfoType::kPath);
    const QString &filePath = fileInfo->pathOf(PathInfoType::kFilePath);

    if (dirPath.isEmpty() || filePath.isEmpty())
        return QString();

    if (isIconCachePath(dirPath))
        return filePath;

    const QString thumbnailName = dataToMd5Hex((QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded)).toLocal8Bit()) + kFormat;
    QString thumbnail = DFMIO::DFMUtils::buildFilePath(d->sizeToFilePath(size).toStdString().c_str(), thumbnailName.toStdString().c_str(), nullptr);
    if (!DFMIO::DFile(thumbnail).exists()) {
        return QString();
    }

    QImageReader ir(thumbnail, QByteArray(kFormat).mid(1));
    if (!ir.canRead()) {
        LocalFileHandler().deleteFileRecursive(QUrl::fromLocalFile(thumbnail));
        return QPixmap();
    }
    ir.setAutoDetectImageFormat(false);

    const QImage image = ir.read();
    const qint64 fileModify = fileInfo->timeOf(TimeInfoType::kLastModifiedSecond).value<qint64>();
    if (!image.isNull() && image.text(QT_STRINGIFY(Thumb::MTime)).toInt() != static_cast<int>(fileModify)) {
        LocalFileHandler().deleteFileRecursive(QUrl::fromLocalFile(thumbnail));

        return QPixmap();
    }

    return QPixmap::fromImage(image);
}

static QString generalKey(const QString &key)
{
    const QStringList &tmp = key.split('/');

    if (tmp.size() > 1)
        return tmp.first() + "/*";

    return key;
}

QString ThumbnailProvider::createThumbnail(const QUrl &url, ThumbnailProvider::Size size)
{
    d->errorString.clear();

    const FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url);

    const QString &dirPath = fileInfo->pathOf(PathInfoType::kAbsolutePath);
    const QString &filePath = fileInfo->pathOf(PathInfoType::kAbsoluteFilePath);

    if (isIconCachePath(dirPath))
        return filePath;

    if (!hasThumbnail(url)) {
        d->errorString = QStringLiteral("This file has not support thumbnail: ") + filePath;

        //!Warnning: Do not store thumbnails to the fail path
        return QString();
    }

    const QString fileUrl = QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded);
    const QString thumbnailName = dataToMd5Hex(fileUrl.toLocal8Bit()) + kFormat;

    // the file is in fail path
    QString thumbnail = DFMIO::DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kThumbnailFailPath).toStdString().c_str(),
                                                       thumbnailName.toStdString().c_str(), nullptr);

    QMimeType mime = d->mimeDatabase.mimeTypeForFile(url);
    QScopedPointer<QImage> image(new QImage());

    QStringList mimeTypeList = { mime.name() };
    mimeTypeList.append(mime.parentMimeTypes());

    //! 新增djvu格式文件缩略图预览
    if (mime.name().contains(DFMGLOBAL_NAMESPACE::Mime::kTypeImageVDjvu)) {
        if (createImageVDjvuThumbnail(filePath, size, image, thumbnailName, thumbnail))
            return thumbnail;
    } else if (mime.name().startsWith("image/")) {
        createImageThumbnail(url, mime, filePath, size, image);
    } else if (mime.name() == DFMGLOBAL_NAMESPACE::Mime::kTypeTextPlain) {
        createTextThumbnail(filePath, size, image);
    } else if (mimeTypeList.contains(DFMGLOBAL_NAMESPACE::Mime::kTypeAppPdf)) {
        createPdfThumbnail(filePath, size, image);
    } else if (mime.name().startsWith("audio/")) {
        createAudioThumbnail(filePath, size, image);
    } else {
        if (createDefaultThumbnail(mime, filePath, size, image, thumbnail))
            return thumbnail;
    }

    // successful
    if (d->errorString.isEmpty()) {
        thumbnail = DFMIO::DFMUtils::buildFilePath(d->sizeToFilePath(size).toStdString().c_str(), thumbnailName.toStdString().c_str(), nullptr);
    } else {
        //fail
        image.reset(new QImage(1, 1, QImage::Format_Mono));
    }

    if (thumbnail.isEmpty())
        thumbnail = DFMIO::DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kThumbnailLargePath).toStdString().c_str(),
                                                                           thumbnailName.toStdString().c_str(), nullptr);

    image->setText(QT_STRINGIFY(Thumb::URL), fileUrl);
    const qint64 fileModify = fileInfo->timeOf(TimeInfoType::kLastModifiedSecond).value<qint64>();
    image->setText(QT_STRINGIFY(Thumb::MTime), QString::number(fileModify));

    // create path
    QFileInfo(thumbnail).absoluteDir().mkpath(".");

    if (!image->save(thumbnail, Q_NULLPTR, 50)) {
        d->errorString = QStringLiteral("Can not save image to ") + thumbnail;
    }

    if (d->errorString.isEmpty()) {
        return thumbnail;
    }

    return QString();
}

void ThumbnailProvider::createAudioThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image)
{
    QProcess ffmpeg;
    ffmpeg.start("ffmpeg", QStringList() << "-nostats"
                                         << "-loglevel"
                                         << "0"
                                         << "-i" << QDir::toNativeSeparators(filePath) << "-an"
                                         << "-vf"
                                         << "scale='min(" + QString::number(size) + ",iw)':-1"
                                         << "-f"
                                         << "image2pipe"
                                         << "-fs"
                                         << "9000"
                                         << "-",
                 QIODevice::ReadOnly);

    if (!ffmpeg.waitForFinished()) {
        d->errorString = ffmpeg.errorString();
        return;
    }

    const QByteArray &output = ffmpeg.readAllStandardOutput();

    if (image->loadFromData(output)) {
        d->errorString.clear();
    } else {
        d->errorString = QString("load image failed from the ffmpeg application");
    }
}

bool ThumbnailProvider::createImageVDjvuThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image,
                                                  const QString &thumbnailName, QString &thumbnail)
{
    thumbnail = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->createThumbnail(QFileInfo(filePath),
                                                                                   static_cast<DTK_GUI_NAMESPACE::DThumbnailProvider::Size>(size));
    d->errorString = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->errorString();

    if (d->errorString.isEmpty()) {
        return true;
    } else {
        const QString &readerBinary = QStandardPaths::findExecutable("deepin-reader");
        if (readerBinary.isEmpty())
            return true;
        //! 使用子进程来调用deepin-reader程序生成djvu格式文件缩略图
        QProcess process;
        QStringList arguments;
        //! 生成缩略图缓存地址
        const QString &saveImage = DFMIO::DFMUtils::buildFilePath(d->sizeToFilePath(size).toStdString().c_str(),
                                                                  thumbnailName.toStdString().c_str(), nullptr);
        arguments << "--thumbnail"
                  << "-f" << filePath << "-t" << saveImage;
        process.start(readerBinary, arguments);

        if (!process.waitForFinished()) {
            d->errorString = process.errorString();

            return false;
        }

        if (process.exitCode() != 0) {
            const QString &error = process.readAllStandardError();

            if (error.isEmpty()) {
                d->errorString = QString("get thumbnail failed from the \"%1\" application").arg(readerBinary);
            } else {
                d->errorString = error;
            }

            return false;
        }

        DFMIO::DFile dfile(saveImage);
        if (dfile.open(DFMIO::DFile::OpenFlag::kReadOnly)) {
            const QByteArray &output = dfile.readAll();
            if (output.isEmpty())
                return false;

            if (image->loadFromData(output, "png")) {
                d->errorString.clear();
            }
            dfile.close();
        }
    }
    return false;
}

void ThumbnailProvider::createImageThumbnail(const QUrl &url, const QMimeType &mime, const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image)
{
    //! fix bug#49451 因为使用mime.preferredSuffix(),会导致后续image.save崩溃，具体原因还需进一步跟进
    //! QImageReader构造时不传format参数，让其自行判断
    //! fix bug #53200 QImageReader构造时不传format参数，会造成没有读取不了真实的文件 类型比如将png图标后缀修改为jpg，读取的类型不对

    QString mimeType = d->mimeDatabase.mimeTypeForFile(url, QMimeDatabase::MatchContent).name();
    QString suffix = mimeType.replace("image/", "");

    QImageReader reader(filePath, suffix.toLatin1());
    if (!reader.canRead()) {
        d->errorString = reader.errorString();
        return;
    }

    const QSize &imageSize = reader.size();

    //fix 读取损坏icns文件（可能任意损坏的image类文件也有此情况）在arm平台上会导致递归循环的问题
    //这里先对损坏文件（imagesize无效）做处理，不再尝试读取其image数据
    if (!imageSize.isValid()) {
        d->errorString = "Fail to read image file attribute data:" + filePath;
        return;
    }

    if (imageSize.width() > size || imageSize.height() > size || mime.name() == DFMGLOBAL_NAMESPACE::Mime::kTypeImageSvgXml) {
        reader.setScaledSize(reader.size().scaled(size, size, Qt::KeepAspectRatio));
    }

    reader.setAutoTransform(true);

    if (!reader.read(image.data())) {
        d->errorString = reader.errorString();
        return;
    }

    if (image->width() > size || image->height() > size) {
        image->operator=(image->scaled(size, size, Qt::KeepAspectRatio));
    }
}

void ThumbnailProvider::createTextThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image)
{
    //FIXME(zccrs): This should be done using the image plugin?
    DFMIO::DFile dfile(filePath);
    if (!dfile.open(DFMIO::DFile::OpenFlag::kReadOnly)) {
        d->errorString = dfile.lastError().errorMsg();
        return;
    }
    FileInfoPointer fileinfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    if (!fileinfo)
        return;

    QString text { FileUtils::toUnicode(dfile.read(2000), fileinfo->nameOf(NameInfoType::kFileName)) };

    QFont font;
    font.setPixelSize(12);

    QPen pen;
    pen.setColor(Qt::black);

    *image = QImage(static_cast<int>(0.70707070 * size), size, QImage::Format_ARGB32_Premultiplied);
    image->fill(Qt::white);

    QPainter painter(image.data());
    painter.setFont(font);
    painter.setPen(pen);

    QTextOption option;

    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    painter.drawText(image->rect(), text, option);
}

void ThumbnailProvider::createPdfThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image)
{
    //FIXME(zccrs): This should be done using the image plugin?
    QScopedPointer<poppler::document> doc(poppler::document::load_from_file(filePath.toStdString()));

    if (!doc || doc->is_locked()) {
        d->errorString = QStringLiteral("Cannot read this pdf file: ") + filePath;
        return;
    }

    if (doc->pages() < 1) {
        d->errorString = QStringLiteral("This stream is invalid");
        return;
    }

    QScopedPointer<const poppler::page> page(doc->create_page(0));

    if (!page) {
        d->errorString = QStringLiteral("Cannot get this page at index 0");
        return;
    }

    poppler::page_renderer pr;
    pr.set_render_hint(poppler::page_renderer::antialiasing, true);
    pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);

    poppler::image imageData = pr.render_page(page.data(), 72, 72, -1, -1, -1, size);

    if (!imageData.is_valid()) {
        d->errorString = QStringLiteral("Render error");
        return;
    }

    poppler::image::format_enum format = imageData.format();
    QImage img;

    switch (format) {
    case poppler::image::format_invalid:
        d->errorString = QStringLiteral("Image format is invalid");
        return;
    case poppler::image::format_mono:
        img = QImage(reinterpret_cast<uchar *>(imageData.data()), imageData.width(), imageData.height(), QImage::Format_Mono);
        break;
    case poppler::image::format_rgb24:
        img = QImage(reinterpret_cast<uchar *>(imageData.data()), imageData.width(), imageData.height(), QImage::Format_ARGB6666_Premultiplied);
        break;
    case poppler::image::format_argb32:
        img = QImage(reinterpret_cast<uchar *>(imageData.data()), imageData.width(), imageData.height(), QImage::Format_ARGB32);
        break;
    default:
        break;
    }

    if (img.isNull()) {
        d->errorString = QStringLiteral("Render error");
        return;
    }

    *image = img.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

bool ThumbnailProvider::createDefaultThumbnail(const QMimeType &mime, const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image, QString &thumbnail)
{
    if (createThumnailByMovieLib(filePath, image))
        return false;

    thumbnail = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->createThumbnail(QFileInfo(filePath), static_cast<DTK_GUI_NAMESPACE::DThumbnailProvider::Size>(size));
    d->errorString = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->errorString();

    if (d->errorString.isEmpty()) {
        return true;
    }

    if (!createThumnailByDtkTools(mime, size, filePath, image))
        return createThumnailByTools(mime, size, filePath, image);

    return false;
}

bool ThumbnailProvider::createThumnailByMovieLib(const QString &filePath, QScopedPointer<QImage> &image)
{
    //获取缩略图生成库函数getMovieCover的指针
    if (!d->libMovieViewer || !d->libMovieViewer->isLoaded()) {
        d->libMovieViewer = new QLibrary("libimageviewer.so");
        d->libMovieViewer->load();
    }
    if (d->libMovieViewer && d->libMovieViewer->isLoaded()) {
        typedef void (*getMovieCover)(const QUrl &url, const QString &savePath, QImage *imageRet);
        getMovieCover func = reinterpret_cast<void (*)(const QUrl &, const QString &, QImage *)>(d->libMovieViewer->resolve("getMovieCover"));
        if (func) {   //存在导出函数getMovieCover
            auto url = QUrl::fromLocalFile(filePath);
            QImage img;
            func(url, filePath, &img);   //调用getMovieCover生成缩略图
            if (!img.isNull()) {
                *image = img;
                d->errorString.clear();
                return true;
            }
        }
    }
    return false;
}

void ThumbnailProvider::initThumnailTool()
{
#ifdef THUMBNAIL_TOOL_DIR
    if (d->keyToThumbnailTool.isEmpty()) {
        d->keyToThumbnailTool["Initialized"] = QString();

        for (const QString &path : QString(THUMBNAIL_TOOL_DIR).split(":")) {
            const QString &thumbnailToolPath = DFMIO::DFMUtils::buildFilePath(path.toStdString().c_str(), "thumbnail", nullptr);
            QDirIterator dir(thumbnailToolPath, { "*.json" }, QDir::NoDotAndDotDot | QDir::Files);

            while (dir.hasNext()) {
                const QString &filePath = dir.next();
                const QFileInfo &fileInfo = dir.fileInfo();

                QFile file(filePath);

                if (!file.open(QFile::ReadOnly)) {
                    continue;
                }

                const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
                file.close();

                const QStringList keys = document.object().toVariantMap().value("Keys").toStringList();
                const QString &toolFilePath = fileInfo.absoluteDir().filePath(fileInfo.baseName());

                if (!DFMIO::DFile(toolFilePath).exists()) {
                    continue;
                }

                for (const QString &key : keys) {
                    if (d->keyToThumbnailTool.contains(key))
                        continue;

                    d->keyToThumbnailTool[key] = toolFilePath;
                }
            }
        }
    }
#endif
}

bool ThumbnailProvider::createThumnailByDtkTools(const QMimeType &mime, ThumbnailProvider::Size size, const QString &filePath, QScopedPointer<QImage> &image)
{
    DFMBASE_NAMESPACE::VideoThumbnailProvider videoProvider;

    QString mimeName = mime.name();
    bool useVideo = videoProvider.hasKey(mimeName);
    if (!useVideo) {
        mimeName = generalKey(mimeName);
        useVideo = videoProvider.hasKey(mimeName);
    }
    if (useVideo) {
        *image = videoProvider.createThumbnail(QString::number(size), filePath);
        d->errorString.clear();
        return true;
    }

    return false;
}

bool ThumbnailProvider::createThumnailByTools(const QMimeType &mime, ThumbnailProvider::Size size, const QString &filePath, QScopedPointer<QImage> &image)
{
    initThumnailTool();
    QString mimeName = mime.name();
    QString tool = d->keyToThumbnailTool.value(mimeName);

    if (tool.isEmpty()) {
        mimeName = generalKey(mimeName);
        tool = d->keyToThumbnailTool.value(mimeName);
    }

    if (tool.isEmpty()) {
        return true;
    }

    QProcess process;
    process.start(tool, { QString::number(size), filePath }, QIODevice::ReadOnly);

    if (!process.waitForFinished()) {
        d->errorString = process.errorString();

        return false;
    }

    if (process.exitCode() != 0) {
        const QString &error = process.readAllStandardError();

        if (error.isEmpty()) {
            d->errorString = QString("get thumbnail failed from the \"%1\" application").arg(tool);
        } else {
            d->errorString = error;
        }

        return false;
    }

    const QByteArray output = process.readAllStandardOutput();
    const QByteArray pngData = QByteArray::fromBase64(output);
    Q_ASSERT(!pngData.isEmpty());

    if (image->loadFromData(pngData, "png")) {
        d->errorString.clear();
    } else {
        // 过滤video tool的其他输出信息
        QString processResult(output);
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
        processResult = processResult.split(QRegExp("[\n]"), QString::SkipEmptyParts).last();
#else
        processResult = processResult.split(QRegExp("[\n]"), Qt::SkipEmptyParts).last();
#endif
        const QByteArray pngData = QByteArray::fromBase64(processResult.toUtf8());
        Q_ASSERT(!pngData.isEmpty());
        if (image->loadFromData(pngData, "png")) {
            d->errorString.clear();
        } else {
            d->errorString = QString("load png image failed from the \"%1\" application").arg(tool);
        }
    }
    return false;
}

bool ThumbnailProvider::isIconCachePath(const QString &dirPath) const
{
    // 判断目录inode值，是否为缓存目录
    // fix: 用户通过数据盘或软链接访问缓存目录时无限生成缩略图的bug
    uint64_t dirInode = d->filePathToInode(dirPath);
    uint64_t smallIconPathInode = d->filePathToInode(d->sizeToFilePath(ThumbnailProvider::Size::kSmall));
    uint64_t normalIconPathInode = d->filePathToInode(d->sizeToFilePath(ThumbnailProvider::Size::kNormal));
    uint64_t largeIconPathInode = d->filePathToInode(d->sizeToFilePath(ThumbnailProvider::Size::kLarge));
    uint64_t failIconPathInode = d->filePathToInode(StandardPaths::location(StandardPaths::kThumbnailFailPath));
    if ((smallIconPathInode != 0 && dirInode == smallIconPathInode)
        || (normalIconPathInode != 0 && dirInode == normalIconPathInode)
        || (largeIconPathInode != 0 && dirInode == largeIconPathInode)
        || (failIconPathInode != 0 && dirInode == failIconPathInode)) {
        return true;
    }
    return false;
}

QString ThumbnailProvider::errorString() const
{
    return d->errorString;
}

qint64 ThumbnailProvider::sizeLimit(const QMimeType &mimeType) const
{
    return d->sizeLimitHash.value(mimeType, d->defaultSizeLimit);
}

ThumbnailProvider::ThumbnailProvider(QObject *parent)
    : QObject(parent), d(new ThumbnailProviderPrivate(this))
{
    d->init();
}

ThumbnailProvider::~ThumbnailProvider()
{
    if (d->libMovieViewer && d->libMovieViewer->isLoaded()) {
        d->libMovieViewer->unload();
        delete d->libMovieViewer;
        d->libMovieViewer = nullptr;
    }
}

}
