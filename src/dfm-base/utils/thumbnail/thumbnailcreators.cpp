// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailcreators.h"
#include "thumbnailhelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-io/dfmio_utils.h>

#include <DThumbnailProvider>

#include <QProcess>
#include <QFont>
#include <QPen>
#include <QPainter>
#include <QImageReader>
#include <QDebug>
#include <QTemporaryDir>

#include <libheif/heif.h>
#include <QImage>
#include <QBuffer>

// use original poppler api
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

#include <appimage/appimage.h>

static constexpr char kFormat[] { ".png" };

using namespace dfmbase;
DFMGLOBAL_USE_NAMESPACE

namespace {
DTK_GUI_NAMESPACE::DThumbnailProvider::Size providerSizeFor(ThumbnailSize size)
{
    switch (size) {
    case ThumbnailSize::kSmall:
        return DTK_GUI_NAMESPACE::DThumbnailProvider::Small;
    case ThumbnailSize::kNormal:
        return DTK_GUI_NAMESPACE::DThumbnailProvider::Normal;
    case ThumbnailSize::kLarge:
        return DTK_GUI_NAMESPACE::DThumbnailProvider::Large;
    case ThumbnailSize::kXLarge:
        return DTK_GUI_NAMESPACE::DThumbnailProvider::Large;
    default:
        return DTK_GUI_NAMESPACE::DThumbnailProvider::Large;
    }
}
}   // namespace

QImage decodeHeifThumbnail(const QString &filePath, int maxSize)
{
    heif_context *ctx = heif_context_alloc();
    if (!ctx) {
        qWarning() << "HEIF: Failed to allocate context.";
        return {};
    }

    heif_error err = heif_context_read_from_file(ctx, filePath.toUtf8().constData(), nullptr);
    if (err.code != heif_error_Ok) {
        qWarning() << "HEIF: Failed to read file:" << filePath << "Error:" << err.message;
        heif_context_free(ctx);
        return {};
    }

    heif_image_handle *handle = nullptr;
    err = heif_context_get_primary_image_handle(ctx, &handle);
    if (err.code != heif_error_Ok) {
        qWarning() << "HEIF: Failed to get image handle:" << filePath << "Error:" << err.message;
        heif_context_free(ctx);
        return {};
    }

    heif_image *img = nullptr;

    // Check for alpha channel and decode appropriately
    bool hasAlpha = heif_image_handle_has_alpha_channel(handle);
    heif_colorspace cs = heif_colorspace_RGB;
    heif_chroma chroma = hasAlpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB;

    err = heif_decode_image(handle, &img, cs, chroma, nullptr);
    if (err.code != heif_error_Ok || !img) {
        qWarning() << "HEIF: Failed to decode image:" << filePath << "Error:" << err.message;
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return {};
    }

    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);

    if (width <= 0 || height <= 0) {
        qWarning() << "HEIF: Invalid image dimensions:" << width << "x" << height;
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return {};
    }

    int stride = 0;
    const uint8_t *data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
    if (!data || stride <= 0) {
        qWarning() << "HEIF: Failed to get image plane data.";
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return {};
    }

    QImage::Format format = hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888;
    QImage image(data, width, height, stride, format);

    // Copy image to detach from HEIF buffer
    QImage finalImage = image.copy();

    // Optionally scale if larger than requested thumbnail size
    if (finalImage.width() > maxSize || finalImage.height() > maxSize) {
        finalImage = finalImage.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return finalImage;
}
QImage ThumbnailCreators::defaultThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: using default creator for:" << filePath << "size:" << size;

    QFileInfo qInf(filePath);
    if (size > ThumbnailSize::kLarge) {
        qCInfo(logDFMBase) << "thumbnail: requested size" << size
                           << "is not supported by DThumbnailProvider, downgrade to Large";
    }

    auto sz = providerSizeFor(size);
    QString thumbPath = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->createThumbnail(qInf, sz);
    if (thumbPath.isEmpty()) {
        qCWarning(logDFMBase) << "thumbnail: default creator failed for:" << filePath;
        qCWarning(logDFMBase) << "thumbnail: DThumbnailProvider error:" << DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->errorString();
        return {};
    }

    qCDebug(logDFMBase) << "thumbnail: default creator succeeded, thumbnail path:" << thumbPath;
    return QImage(thumbPath);
}

QImage ThumbnailCreators::videoThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: creating video thumbnail for:" << filePath;

    QImage img = videoThumbnailCreatorLib(filePath, size);
    if (img.isNull()) {
        qCDebug(logDFMBase) << "thumbnail: video library creator failed, trying ffmpeg for:" << filePath;
        img = videoThumbnailCreatorFfmpeg(filePath, size);
    } else {
        qCDebug(logDFMBase) << "thumbnail: video library creator succeeded for:" << filePath;
    }

    return img;
}

QImage ThumbnailCreators::videoThumbnailCreatorFfmpeg(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: using ffmpeg for video:" << filePath << "size:" << size;

    // Probe for duration
    QProcess probe;
    QStringList probeArgs {
        "-v", "error",
        "-show_entries", "format=duration",
        "-of", "default=noprint_wrappers=1:nokey=1",
        filePath
    };
    probe.start("ffprobe", probeArgs);
    if (!probe.waitForFinished(5000)) {
        probe.kill();
        return QImage();
    }

    bool ok = false;
    double duration = probe.readAllStandardOutput().trimmed().toDouble(&ok);
    if (!ok || duration <= 0.0)
        return QImage();

    double midpoint = duration / 2.0;
    QString midpointStr = QString::number(midpoint, 'f', 2);
    QString scaleFilter = QString("scale='min(%1,iw)':-1").arg(size);

    QProcess ffmpeg;
    QStringList args {
        "-ss", midpointStr,
        "-i", filePath,
        "-vf", scaleFilter,
        "-frames:v", "1",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-"
    };

    ffmpeg.start("ffmpeg", args, QIODevice::ReadOnly);
    if (!ffmpeg.waitForFinished()) {
        qCWarning(logDFMBase) << "thumbnail: ffmpeg execution failed for:" << filePath
                              << "error:" << ffmpeg.errorString();
        return QImage();
    }

    QByteArray data = ffmpeg.readAllStandardOutput();
    QImage img;
    if (!img.loadFromData(data, "PNG")) {
        qCWarning(logDFMBase) << "thumbnail: failed to load image from ffmpeg output for:" << filePath;
    } else {
        qCDebug(logDFMBase) << "thumbnail: ffmpeg video thumbnail created successfully for:" << filePath;
    }

    return img;
}

QImage ThumbnailCreators::videoThumbnailCreatorLib(const QString &filePath, ThumbnailSize size)
{
    Q_UNUSED(size)

    qCDebug(logDFMBase) << "thumbnail: using video library for:" << filePath;

    QImage img;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    static QLibrary lib("libimageviewer.so");
#else
    static QLibrary lib("libimageviewer6.so");
#endif

    if (lib.isLoaded() || lib.load()) {
        typedef void (*GetMovieCover)(const QUrl &, const QString &, QImage *);
        GetMovieCover func = reinterpret_cast<GetMovieCover>(lib.resolve("getMovieCover"));

        if (func) {
            qCDebug(logDFMBase) << "thumbnail: calling getMovieCover function for:" << filePath;
            func(QUrl::fromLocalFile(filePath), filePath, &img);
            if (!img.isNull()) {
                qCDebug(logDFMBase) << "thumbnail: video library thumbnail created successfully for:" << filePath;
            }
        } else {
            qCWarning(logDFMBase) << "thumbnail: failed to resolve getMovieCover function from:" << lib.fileName();
        }
    } else {
        qCWarning(logDFMBase) << "thumbnail: failed to load video library:" << lib.fileName() << "error:" << lib.errorString();
    }

    return img;
}

QImage ThumbnailCreators::textThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: creating text thumbnail for:" << filePath << "size:" << size;

    QImage img;
    DFMIO::DFile dfile(filePath);
    if (!dfile.open(DFMIO::DFile::OpenFlag::kReadOnly)) {
        qCWarning(logDFMBase) << "thumbnail: failed to open text file:" << filePath;
        return img;
    }

    FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    if (!info) {
        qCWarning(logDFMBase) << "thumbnail: failed to create FileInfo for text file:" << filePath;
        return img;
    }

    QString text { FileUtils::toUnicode(dfile.read(2000), info->nameOf(NameInfoType::kFileName)) };
    qCDebug(logDFMBase) << "thumbnail: read" << text.length() << "characters from text file:" << filePath;

    QFont font;
    font.setPixelSize(12);

    QPen pen;
    pen.setColor(Qt::black);

    img = QImage(static_cast<int>(0.70707070 * size), size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setFont(font);
    painter.setPen(pen);

    QTextOption option;
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    painter.drawText(img.rect(), text, option);

    qCDebug(logDFMBase) << "thumbnail: text thumbnail created successfully for:" << filePath;
    return img;
}

QImage ThumbnailCreators::audioThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: creating audio thumbnail for:" << filePath << "size:" << size;

    QProcess ffmpeg;
    QStringList args { "-nostats", "-loglevel", "0", "-i", filePath,
                       "-an", "-vf", QString("scale='min(%1, iw)':-1").arg(size), "-f", "image2pipe", "-fs", "9000", "-" };
    ffmpeg.start("ffmpeg", args, QIODevice::ReadOnly);

    QImage img;
    if (!ffmpeg.waitForFinished()) {
        qCWarning(logDFMBase) << "thumbnail: ffmpeg execution failed for audio file:" << filePath
                              << "error:" << ffmpeg.errorString();
        return img;
    }

    const QByteArray &output = ffmpeg.readAllStandardOutput();
    if (output.isEmpty()) {
        qCDebug(logDFMBase) << "thumbnail: no embedded artwork found in audio file:" << filePath;
        return img;
    }

    if (!img.loadFromData(output)) {
        qCWarning(logDFMBase) << "thumbnail: failed to load embedded artwork from audio file:" << filePath;
    } else {
        qCDebug(logDFMBase) << "thumbnail: audio thumbnail created successfully for:" << filePath;
    }

    return img;
}

QImage ThumbnailCreators::imageThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    //! fix bug#49451 因为使用mime.preferredSuffix(),会导致后续image.save崩溃，具体原因还需进一步跟进
    //! QImageReader构造时不传format参数，让其自行判断
    //! fix bug #53200 QImageReader构造时不传format参数，会造成没有读取不了真实的文件 类型比如将png图标后缀修改为jpg，读取的类型不对

    QString mimeType = DMimeDatabase().mimeTypeForFile(QUrl::fromLocalFile(filePath), QMimeDatabase::MatchContent).name();

    if (mimeType == "image/heif" || mimeType == "image/heic") {
        QImage heifImage = decodeHeifThumbnail(filePath, size);
        if (!heifImage.isNull()) {
            qCDebug(logDFMBase) << "thumbnail: HEIF thumbnail decoded natively for:" << filePath;
            return heifImage;
        } else {
            qCWarning(logDFMBase) << "thumbnail: native HEIF decoding failed for:" << filePath;
        }
    }

    const QString &suffix = mimeType.replace("image/", "");

    QImageReader reader(filePath, suffix.toLatin1());
    if (!reader.canRead()) {
        qCWarning(logDFMBase) << "thumbnail: cannot read image file:" << filePath
                              << "error:" << reader.errorString();
        return {};
    }

    const QSize &imageSize = reader.size();

    // fix 读取损坏icns文件（可能任意损坏的image类文件也有此情况）在arm平台上会导致递归循环的问题
    // 这里先对损坏文件（imagesize无效）做处理，不再尝试读取其image数据
    if (!imageSize.isValid()) {
        qCWarning(logDFMBase) << "thumbnail: image file has invalid size attributes:" << filePath;
        return {};
    }

    qCDebug(logDFMBase) << "thumbnail: image file size:" << imageSize << "for:" << filePath;

    const QString &defaultMime = DMimeDatabase().mimeTypeForFile(QUrl::fromLocalFile(filePath)).name();
    if (imageSize.width() > size || imageSize.height() > size || defaultMime == DFMGLOBAL_NAMESPACE::Mime::kTypeImageSvgXml) {
        qCDebug(logDFMBase) << "thumbnail: scaling image from" << imageSize << "to fit size:" << size;
        reader.setScaledSize(reader.size().scaled(size, size, Qt::KeepAspectRatio));
    }

    reader.setAutoTransform(true);
    QImage image;
    if (!reader.read(&image)) {
        qCWarning(logDFMBase) << "thumbnail: failed to read image file:" << filePath
                              << "error:" << reader.errorString();
        return image;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    image = FileUtils::convertToSRgbColorSpace(image);
#endif

    qCDebug(logDFMBase) << "thumbnail: image thumbnail created successfully for:" << filePath;
    return image;
}

QImage ThumbnailCreators::djvuThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: creating djvu thumbnail for:" << filePath << "size:" << size;

    QImage img = defaultThumbnailCreator(filePath, size);
    if (!img.isNull()) {
        qCDebug(logDFMBase) << "thumbnail: djvu thumbnail created by default creator for:" << filePath;
        return img;
    }

    QString program;
    QStringList arguments;

    // ------------------------------------------------------------------
    // First try dpkg deepin-reader (old systems)
    // ------------------------------------------------------------------
    const QString readerBinary = QStandardPaths::findExecutable("deepin-reader");
    if (!readerBinary.isEmpty()) {
        program = readerBinary;
        qCDebug(logDFMBase) << "thumbnail: using dpkg deepin-reader";

    } else {
        // ------------------------------------------------------------------
        // Fall back to Linyaps version
        // ------------------------------------------------------------------
        const QString llCli = QStandardPaths::findExecutable("ll-cli");
        if (llCli.isEmpty()) {
            qCWarning(logDFMBase) << "thumbnail: neither deepin-reader nor ll-cli found";
            return img;
        }

        program = llCli;

        QString appId = "org.deepin.reader";

        QProcess detect;
        detect.start(llCli, { "list" });
        detect.waitForFinished();

        const QString output = detect.readAllStandardOutput();

        if (!output.contains(appId)) {
            qCWarning(logDFMBase) << "thumbnail: deepin-reader linyaps app not found";
            return img;
        }

        qCDebug(logDFMBase) << "thumbnail: using linyaps deepin-reader, appId:" << appId;

        arguments << "run"
                  << appId
                  << "--"
                  << "deepin-reader";
    }

    const QString fileUrl = QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded);
    const QString thumbnailName = ThumbnailHelper::dataToMd5Hex(fileUrl.toLocal8Bit()) + kFormat;
    const QString saveImage = DFMIO::DFMUtils::buildFilePath(
            ThumbnailHelper::sizeToFilePath(size).toStdString().c_str(),
            thumbnailName.toStdString().c_str(),
            nullptr);

    arguments << "--thumbnail"
              << "-f" << filePath
              << "-t" << saveImage;

    QProcess process;
    process.start(program, arguments);

    if (!process.waitForFinished() || process.exitCode() != 0) {
        qCWarning(logDFMBase) << "thumbnail: deepin-reader execution failed for:" << filePath
                              << "error:" << process.errorString()
                              << "exit code:" << process.exitCode();
        return img;
    }

    qCDebug(logDFMBase) << "thumbnail: deepin-reader completed successfully";

    DFMIO::DFile dfile(saveImage);
    if (!dfile.open(DFMIO::DFile::OpenFlag::kReadOnly)) {
        qCWarning(logDFMBase) << "thumbnail: failed to open generated thumbnail:" << saveImage;
        return img;
    }

    const QByteArray outputData = dfile.readAll();
    dfile.close();

    if (outputData.isEmpty()) {
        qCWarning(logDFMBase) << "thumbnail: generated thumbnail file is empty";
        return img;
    }

    if (!img.loadFromData(outputData, "png")) {
        qCWarning(logDFMBase) << "thumbnail: failed to load thumbnail data";
    }

    return img;
}

QImage ThumbnailCreators::pdfThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: creating PDF thumbnail for:" << filePath << "size:" << size;

    QImage img;
    QScopedPointer<poppler::document> doc(poppler::document::load_from_file(filePath.toStdString()));
    if (!doc || doc->is_locked()) {
        qCWarning(logDFMBase) << "thumbnail: cannot read PDF file (file not found or locked):" << filePath;
        return img;
    }

    if (doc->pages() < 1) {
        qCWarning(logDFMBase) << "thumbnail: PDF file has no pages:" << filePath;
        return img;
    }

    qCDebug(logDFMBase) << "thumbnail: PDF file has" << doc->pages() << "pages, rendering first page for:" << filePath;

    QScopedPointer<const poppler::page> page(doc->create_page(0));
    if (!page) {
        qCWarning(logDFMBase) << "thumbnail: failed to create page 0 from PDF:" << filePath;
        return img;
    }

    poppler::page_renderer pr;
    pr.set_render_hint(poppler::page_renderer::antialiasing, true);
    pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);

    poppler::image imageData = pr.render_page(page.data(), 72, 72, -1, -1, -1, size);
    if (!imageData.is_valid()) {
        qCWarning(logDFMBase) << "thumbnail: PDF page rendering failed:" << filePath;
        return img;
    }

    poppler::image::format_enum format = imageData.format();
    switch (format) {
    case poppler::image::format_invalid:
        qCWarning(logDFMBase) << "thumbnail: PDF rendered image has invalid format:" << filePath;
        break;
    case poppler::image::format_mono:
        img = QImage(reinterpret_cast<uchar *>(imageData.data()), imageData.width(), imageData.height(), QImage::Format_Mono);
        qCDebug(logDFMBase) << "thumbnail: PDF rendered as mono format:" << filePath;
        break;
    case poppler::image::format_rgb24:
        img = QImage(reinterpret_cast<uchar *>(imageData.data()), imageData.width(), imageData.height(), QImage::Format_ARGB6666_Premultiplied);
        qCDebug(logDFMBase) << "thumbnail: PDF rendered as RGB24 format:" << filePath;
        break;
    case poppler::image::format_argb32:
        img = QImage(reinterpret_cast<uchar *>(imageData.data()), imageData.width(), imageData.height(), QImage::Format_ARGB32);
        qCDebug(logDFMBase) << "thumbnail: PDF rendered as ARGB32 format:" << filePath;
        break;
    default:
        qCWarning(logDFMBase) << "thumbnail: PDF rendered with unknown format:" << format << "for:" << filePath;
        break;
    }

    if (!img.isNull()) {
        img = img.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qCDebug(logDFMBase) << "thumbnail: PDF thumbnail created successfully for:" << filePath;
    }

    return img;
}

QImage ThumbnailCreators::appimageThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: creating AppImage thumbnail for:" << filePath << "size:" << size;

    // 1. Check if AppImage exists
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    if (!info || !info->exists()) {
        qCWarning(logDFMBase) << "thumbnail: AppImage file not found:" << filePath;
        return QImage();
    }

    // 2. Check if file is AppImage type and has executable permission
    if (info->nameOf(NameInfoType::kMimeTypeName) != Global::Mime::kTypeAppAppimage
        || !info->isAttributes(FileInfo::FileIsType::kIsExecutable)) {
        qCWarning(logDFMBase) << "thumbnail: file is not a valid AppImage or lacks executable permission:" << filePath
                              << "mimeType:" << (info ? info->nameOf(NameInfoType::kMimeTypeName) : "null")
                              << "isExecutable:" << (info ? info->isAttributes(FileInfo::FileIsType::kIsExecutable) : false);
        return QImage();
    }

    // 3. Use libappimage to read .DirIcon directly from AppImage
    unsigned long iconSize = 0;
    char *iconBuffer = nullptr;

    // Read .DirIcon file from AppImage without extraction
    bool success = appimage_read_file_into_buffer_following_symlinks(
            filePath.toUtf8().constData(),
            ".DirIcon",
            &iconBuffer,
            &iconSize);

    // Use RAII to ensure buffer cleanup
    QScopedPointer<char, QScopedPointerPodDeleter> bufferCleanup(iconBuffer);

    if (!success || !iconBuffer || iconSize == 0) {
        qCWarning(logDFMBase) << "thumbnail: failed to read .DirIcon from AppImage:" << filePath;
        return QImage();
    }

    qCDebug(logDFMBase) << "thumbnail: successfully read .DirIcon from AppImage, size:" << iconSize << "bytes";

    // 4. Load image from buffer
    QImage icon;
    if (!icon.loadFromData(reinterpret_cast<const uchar *>(iconBuffer), iconSize)) {
        qCWarning(logDFMBase) << "thumbnail: failed to load image data from .DirIcon for:" << filePath;
        return QImage();
    }

    // 5. Scale image to requested thumbnail size
    if (icon.width() > size || icon.height() > size) {
        icon = icon.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qCDebug(logDFMBase) << "thumbnail: scaled AppImage icon to" << size << "x" << size;
    }

    qCDebug(logDFMBase) << "thumbnail: AppImage thumbnail created successfully for:" << filePath;
    return icon;
}

QImage ThumbnailCreators::pptxThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCInfo(logDFMBase) << "thumbnail: creating PPTX thumbnail for:" << filePath << "size:" << size;

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    if (!info || !info->exists()) {
        qCWarning(logDFMBase) << "thumbnail: PPTX file not found:" << filePath;
        return QImage();
    }

    // 2. Verify file type
    const QMimeType &mime = info->fileMimeType();
    QStringList candidateTypes { info->nameOf(NameInfoType::kMimeTypeName) };
    candidateTypes.append(mime.parentMimeTypes());
    if (!candidateTypes.contains(Global::Mime::kTypeAppPptx)) {
        qCWarning(logDFMBase) << "thumbnail: file is not a valid PPTX file:" << filePath
                              << "mimeTypes:" << candidateTypes;
        return QImage();
    }

    // 3. Create temporary directory for PPTX extraction
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qCWarning(logDFMBase) << "thumbnail: failed to create temporary directory for PPTX extraction:" << tempDir.errorString()
                              << "file:" << filePath;
        return QImage();
    }

    const QString extractPath = tempDir.path();
    qCDebug(logDFMBase) << "thumbnail: extracting PPTX to temporary directory:" << extractPath;

    // 4. Extract PPTX file using unzip command
    QProcess unzipProcess;
    QStringList arguments;
    arguments << "-q"
              << "-o" << filePath << "-d" << extractPath;
    unzipProcess.start("unzip", arguments);

    if (!unzipProcess.waitForFinished(10000) || unzipProcess.exitCode() != 0) {
        qCWarning(logDFMBase) << "thumbnail: failed to extract PPTX file:" << filePath
                              << "error:" << unzipProcess.errorString()
                              << "exit code:" << unzipProcess.exitCode();
        return QImage();
    }

    qCDebug(logDFMBase) << "thumbnail: PPTX extraction completed successfully for:" << filePath;

    // 5. Look for built-in thumbnail docProps/thumbnail.jpeg
    QString thumbnailPath = extractPath + "/docProps/thumbnail.jpeg";
    if (QFile::exists(thumbnailPath)) {
        qCDebug(logDFMBase) << "thumbnail: found built-in thumbnail in docProps:" << thumbnailPath;
        QImage thumbnail;
        if (thumbnail.load(thumbnailPath)) {
            qCDebug(logDFMBase) << "thumbnail: PPTX thumbnail loaded successfully from docProps/thumbnail.jpeg";
            return thumbnail.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            qCWarning(logDFMBase) << "thumbnail: failed to load built-in thumbnail:" << thumbnailPath;
        }
    } else {
        qCDebug(logDFMBase) << "thumbnail: no built-in thumbnail found in docProps/thumbnail.jpeg for:" << filePath;
    }

    // 6. If no built-in thumbnail found, return empty image
    qCDebug(logDFMBase) << "thumbnail: no thumbnail available for PPTX file:" << filePath;
    return QImage();
}

QImage ThumbnailCreators::uabThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: creating UAB thumbnail for:" << filePath << "size:" << size;

    // 1. Validate file existence
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    if (!info || !info->exists()) {
        qCWarning(logDFMBase) << "thumbnail: UAB file not found:" << filePath;
        return QImage();
    }

    // 2. Verify file type
    if (!info || info->nameOf(NameInfoType::kMimeTypeName) != Global::Mime::kTypeAppUab) {
        qCWarning(logDFMBase) << "thumbnail: file is not a valid UAB file:" << filePath
                              << "mimeType:" << (info ? info->nameOf(NameInfoType::kMimeTypeName) : "null");
        return QImage();
    }

    // 3. Query ELF section information using readelf (read-only, no file modification)
    QProcess readelfProcess;
    readelfProcess.start("readelf", { "-W", "-S", filePath });

    if (!readelfProcess.waitForFinished(5000)) {
        qCWarning(logDFMBase) << "thumbnail: readelf timeout for UAB file:" << filePath;
        readelfProcess.kill();
        return QImage();
    }

    if (readelfProcess.exitCode() != 0) {
        qCWarning(logDFMBase) << "thumbnail: readelf failed for UAB file:" << filePath
                              << "stderr:" << readelfProcess.readAllStandardError();
        return QImage();
    }

    // 4. Parse readelf output to locate linglong.icon section
    QString readelfOutput = readelfProcess.readAllStandardOutput();
    QStringList lines = readelfOutput.split('\n');

    qint64 sectionOffset = -1;
    qint64 sectionSize = -1;
    bool sectionFound = false;

    for (const QString &line : lines) {
        if (line.contains("linglong.icon")) {
            // Parse section header line format:
            // [Nr] Name          Type     Address          Off    Size   ES Flg Lk Inf Al
            // [26] linglong.icon PROGBITS 0000000000000000 002e30 004e20 00   0   0  1
            QString normalizedLine = line.simplified();
            QStringList parts = normalizedLine.split(' ', Qt::SkipEmptyParts);

            // Validate column count (need at least 7 columns)
            if (parts.size() >= 7) {
                bool offsetOk = false, sizeOk = false;
                sectionOffset = parts[4].toLongLong(&offsetOk, 16);   // Column 5: Offset (hex)
                sectionSize = parts[5].toLongLong(&sizeOk, 16);   // Column 6: Size (hex)

                if (offsetOk && sizeOk && sectionOffset > 0 && sectionSize > 0) {
                    sectionFound = true;
                    qCDebug(logDFMBase) << "thumbnail: found linglong.icon section - offset:"
                                        << QString("0x%1").arg(sectionOffset, 0, 16)
                                        << "size:" << sectionSize << "bytes";
                    break;
                }
            }
        }
    }

    if (!sectionFound) {
        qCWarning(logDFMBase) << "thumbnail: linglong.icon section not found in UAB file:" << filePath;
        return QImage();
    }

    // 5. Extract section data using QFile (read-only mode, no file modification)
    QFile uabFile(filePath);
    if (!uabFile.open(QIODevice::ReadOnly)) {
        qCWarning(logDFMBase) << "thumbnail: failed to open UAB file for reading:" << filePath
                              << "error:" << uabFile.errorString();
        return QImage();
    }

    if (!uabFile.seek(sectionOffset)) {
        qCWarning(logDFMBase) << "thumbnail: failed to seek to section offset:" << sectionOffset
                              << "in file:" << filePath
                              << "error:" << uabFile.errorString();
        uabFile.close();
        return QImage();
    }

    QByteArray iconData = uabFile.read(sectionSize);
    uabFile.close();   // Close immediately to release file handle

    if (iconData.size() != sectionSize) {
        qCWarning(logDFMBase) << "thumbnail: incomplete section data read - expected:" << sectionSize
                              << "bytes, got:" << iconData.size() << "bytes for:" << filePath;
        return QImage();
    }

    qCDebug(logDFMBase) << "thumbnail: successfully extracted" << iconData.size()
                        << "bytes of icon data from UAB file:" << filePath;

    // 6. Load image directly from extracted data (no temporary file needed)
    QImage icon;
    if (!icon.loadFromData(iconData)) {
        qCWarning(logDFMBase) << "thumbnail: failed to decode image from extracted icon data for:" << filePath;
        return QImage();
    }

    // 7. Scale image to requested thumbnail size
    if (icon.width() > size || icon.height() > size) {
        icon = icon.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qCDebug(logDFMBase) << "thumbnail: scaled UAB icon from"
                            << icon.size() << "to fit size:" << size;
    }

    qCDebug(logDFMBase) << "thumbnail: UAB thumbnail created successfully for:" << filePath;
    return icon;
}
