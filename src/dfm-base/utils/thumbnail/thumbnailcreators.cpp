// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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

// use original poppler api
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

static constexpr char kFormat[] { ".png" };

using namespace dfmbase;
DFMGLOBAL_USE_NAMESPACE

QImage ThumbnailCreators::defaultThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCDebug(logDFMBase) << "thumbnail: using default creator for:" << filePath << "size:" << size;
    
    QFileInfo qInf(filePath);
    auto sz = static_cast<DTK_GUI_NAMESPACE::DThumbnailProvider::Size>(size);
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

    const QString &readerBinary = QStandardPaths::findExecutable("deepin-reader");
    if (readerBinary.isEmpty()) {
        qCWarning(logDFMBase) << "thumbnail: deepin-reader not found, cannot create djvu thumbnail for:" << filePath;
        return img;
    }
    
    qCDebug(logDFMBase) << "thumbnail: using deepin-reader to create djvu thumbnail for:" << filePath;
    
    // Use subprocess to call deepin-reader program to generate djvu format file thumbnail
    QProcess process;
    QStringList arguments;
    // Generate thumbnail cache address
    const QString &fileUrl = QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded);
    const QString &thumbnailName = ThumbnailHelper::dataToMd5Hex(fileUrl.toLocal8Bit()) + kFormat;
    const QString &saveImage = DFMIO::DFMUtils::buildFilePath(ThumbnailHelper::sizeToFilePath(size).toStdString().c_str(),
                                                              thumbnailName.toStdString().c_str(), nullptr);
    arguments << "--thumbnail"
              << "-f" << filePath << "-t" << saveImage;
    process.start(readerBinary, arguments);

    if (!process.waitForFinished() || process.exitCode() != 0) {
        qCWarning(logDFMBase) << "thumbnail: deepin-reader execution failed for:" << filePath
                              << "error:" << process.errorString()
                              << "exit code:" << process.exitCode();
        return img;
    }

    qCDebug(logDFMBase) << "thumbnail: deepin-reader completed successfully, reading thumbnail from:" << saveImage;

    DFMIO::DFile dfile(saveImage);
    if (dfile.open(DFMIO::DFile::OpenFlag::kReadOnly)) {
        const QByteArray &output = dfile.readAll();
        if (output.isEmpty()) {
            qCWarning(logDFMBase) << "thumbnail: generated thumbnail file is empty:" << saveImage;
            dfile.close();
            return img;
        }

        if (img.loadFromData(output, "png")) {
            qCDebug(logDFMBase) << "thumbnail: djvu thumbnail created successfully for:" << filePath;
        } else {
            qCWarning(logDFMBase) << "thumbnail: failed to load thumbnail data from:" << saveImage;
        }
        dfile.close();
    } else {
        qCWarning(logDFMBase) << "thumbnail: failed to open generated thumbnail file:" << saveImage;
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
    if (!QFile::exists(filePath)) {
        qCWarning(logDFMBase) << "thumbnail: AppImage file not found:" << filePath;
        return QImage();
    }

    // 2. Check if file is AppImage type and has executable permission
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath),
                                              Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!info
        || info->nameOf(NameInfoType::kMimeTypeName) != Global::Mime::kTypeAppAppimage
        || !info->isAttributes(FileInfo::FileIsType::kIsExecutable)) {
        qCWarning(logDFMBase) << "thumbnail: file is not a valid AppImage or lacks executable permission:" << filePath
                              << "mimeType:" << (info ? info->nameOf(NameInfoType::kMimeTypeName) : "null")
                              << "isExecutable:" << (info ? info->isAttributes(FileInfo::FileIsType::kIsExecutable) : false);
        return QImage();
    }

    // 3. Create temporary directory for AppImage extraction
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qCWarning(logDFMBase) << "thumbnail: failed to create temporary directory for AppImage extraction:" << tempDir.errorString()
                              << "file:" << filePath;
        return QImage();
    }
    auto extractTo = tempDir.path();

    qCDebug(logDFMBase) << "thumbnail: extracting AppImage to temporary directory:" << extractTo;

    // 4. Extract AppImage to temporary directory
    QProcess proc;
    proc.setWorkingDirectory(extractTo);
    proc.start(filePath, { "--appimage-extract" });
    auto done = proc.waitForFinished();
    
    if (!done || proc.exitCode() != 0) {
        qCWarning(logDFMBase) << "thumbnail: AppImage extraction failed for:" << filePath
                              << "exit code:" << proc.exitCode()
                              << "error:" << proc.errorString();
        return QImage();
    }
    
    qCDebug(logDFMBase) << "thumbnail: AppImage extraction completed successfully for:" << filePath;

    // 5. Search for PNG/SVG files in temporary directory
    QString iconPath;
    QDir extractDir(extractTo + "/squashfs-root");
    auto files = extractDir.entryInfoList(QStringList { "*.png", "*.svg" },
                                          QDir::Files | QDir::NoDotAndDotDot);
    iconPath = files.isEmpty() ? "" : files.first().filePath();

    QImage icon;
    if (!iconPath.isEmpty()) {
        qCDebug(logDFMBase) << "thumbnail: found AppImage icon:" << iconPath;
        icon = QImage(iconPath).scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!icon.isNull()) {
            qCDebug(logDFMBase) << "thumbnail: AppImage thumbnail created successfully for:" << filePath;
        } else {
            qCWarning(logDFMBase) << "thumbnail: failed to load AppImage icon:" << iconPath;
        }
    } else {
        qCWarning(logDFMBase) << "thumbnail: no icon found in AppImage:" << filePath;
    }

    return icon;
}

QImage ThumbnailCreators::pptxThumbnailCreator(const QString &filePath, ThumbnailSize size)
{
    qCInfo(logDFMBase) << "thumbnail: creating PPTX thumbnail for:" << filePath << "size:" << size;

    // 1. Verify file exists
    if (!QFile::exists(filePath)) {
        qCWarning(logDFMBase) << "thumbnail: PPTX file not found:" << filePath;
        return QImage();
    }

    // 2. Verify file type
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath),
                                              Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!info || info->nameOf(NameInfoType::kMimeTypeName) != Global::Mime::kTypeAppPptx) {
        qCWarning(logDFMBase) << "thumbnail: file is not a valid PPTX file:" << filePath
                              << "mimeType:" << (info ? info->nameOf(NameInfoType::kMimeTypeName) : "null");
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
