/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "dthumbnailprovider.h"
#include "dvideothumbnailprovider.h"
#include "dfm-base/mimetype/dmimedatabase.h"
#include "dfm-base/mimetype/mimetypedisplaymanager.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileoperator.h"

#include <DThumbnailProvider>

#include <QCryptographicHash>
#include <QDir>
#include <QDateTime>
#include <QImageReader>
#include <QQueue>
#include <QMimeType>
#include <QReadWriteLock>
#include <QWaitCondition>
#include <QPainter>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QDebug>
#include <QtConcurrent>

// use original poppler api
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

#include <sys/stat.h>

constexpr char kFormat[] { ".png" };

inline QByteArray dataToMd5Hex(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

DFMBASE_BEGIN_NAMESPACE

class DThumbnailProviderPrivate
{
public:
    explicit DThumbnailProviderPrivate(DThumbnailProvider *qq);

    void init();

    QString sizeToFilePath(DThumbnailProvider::Size size) const;

    DThumbnailProvider *q = nullptr;
    QString errorString;
    qint64 defaultSizeLimit = 1024 * 1024 * 20;
    QHash<QMimeType, qint64> sizeLimitHash;
    DMimeDatabase mimeDatabase;

    static QSet<QString> hasThumbnailMimeHash;

    struct ProduceInfo
    {
        QFileInfo fileInfo;
        DThumbnailProvider::Size size;
        DThumbnailProvider::CallBack callback = nullptr;
    };

    QQueue<ProduceInfo> produceQueue;
    QQueue<QString> produceAbsoluteFilePathQueue;

    bool running = true;

    QWaitCondition waitCondition;
    QMutex mutex;

    QHash<QString, QString> keyToThumbnailTool;
};

class DFileThumbnailProviderPrivate : public DThumbnailProvider
{
};

DFMBASE_END_NAMESPACE

DFMBASE_USE_NAMESPACE

QSet<QString> DThumbnailProviderPrivate::hasThumbnailMimeHash;
Q_GLOBAL_STATIC(DFileThumbnailProviderPrivate, ftpGlobal)

DThumbnailProviderPrivate::DThumbnailProviderPrivate(DThumbnailProvider *qq)
    : q(qq)
{
}

void DThumbnailProviderPrivate::init()
{
    sizeLimitHash.reserve(28);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeTextPlain), 1024 * 1024);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeAppPdf), INT64_MAX);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeAppVRRMedia), INT64_MAX);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeAppVMAsf), INT64_MAX);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeAppMxf), INT64_MAX);

    //images
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImageIef), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImageTiff), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImageXTMultipage), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImageVDMultipage), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImageXADng), 1024 * 1024 * 80);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImageJpeg), 1024 * 1024 * 30);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImagePng), 1024 * 1024 * 30);
    sizeLimitHash.insert(mimeDatabase.mimeTypeForName(DFMGLOBAL_NAMESPACE::kMimeTypeImagePipeg), 1024 * 1024 * 30);
}

QString DThumbnailProviderPrivate::sizeToFilePath(DThumbnailProvider::Size size) const
{
    switch (size) {
    case DThumbnailProvider::Size::kSmall:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailSmallPath);
    case DThumbnailProvider::Size::kNormal:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailNormalPath);
    case DThumbnailProvider::Size::kLarge:
        return StandardPaths::location(StandardPaths::StandardLocation::kThumbnailLargePath);
    }
    return "";
}

DThumbnailProvider *DThumbnailProvider::instance()
{
    return ftpGlobal;
}

bool DThumbnailProvider::hasThumbnail(const QFileInfo &info) const
{
    if (!info.isReadable() || !info.isFile())
        return false;

    qint64 fileSize = info.size();

    if (fileSize <= 0)
        return false;

    const QMimeType &mime = d->mimeDatabase.mimeTypeForFile(info);

    // todo lanxs
    //if (mime.name().startsWith("video/") && FileJob::CopyingFiles.contains(QUrl::fromLocalFile(info.filePath())))
    //if (mime.name().startsWith("video/"))
    //  return false;

    if (mime.name().startsWith("video/") && DFMBASE_NAMESPACE::FileUtils::isGvfsFile(QUrl::fromLocalFile(info.absoluteFilePath())))
        return false;

    if (fileSize > sizeLimit(mime) && !mime.name().startsWith("video/"))
        return false;

    return hasThumbnail(mime);
}

bool DThumbnailProvider::hasThumbnail(const QMimeType &mimeType) const
{
    const QString &mime = mimeType.name();
    QStringList mimeTypeList = { mime };
    mimeTypeList.append(mimeType.parentMimeTypes());

    if (mime.startsWith("image") && !Application::instance()->genericAttribute(Application::kPreviewImage).toBool())
        return false;

    if ((mime.startsWith("video")
         || DFMBASE_NAMESPACE::MimeTypeDisplayManager::supportVideoMimeTypes().contains(mime))
        && !Application::instance()->genericAttribute(Application::kPreviewVideo).toBool())
        return false;

    if (mime == DFMGLOBAL_NAMESPACE::kMimeTypeTextPlain && !Application::instance()->genericAttribute(Application::kPreviewTextFile).toBool())
        return false;

    if (Q_LIKELY(mimeTypeList.contains(DFMGLOBAL_NAMESPACE::kMimeTypeAppPdf)
                 || mime == DFMGLOBAL_NAMESPACE::kMimeTypeAppCRRMedia
                 || mime == DFMGLOBAL_NAMESPACE::kMimeTypeAppMxf)
        && !Application::instance()->genericAttribute(Application::kPreviewDocumentFile).toBool()) {
        return false;
    }

    if (DThumbnailProviderPrivate::hasThumbnailMimeHash.contains(mime))
        return true;

    if (Q_LIKELY(mime.startsWith("image") || mime.startsWith("video/"))) {
        DThumbnailProviderPrivate::hasThumbnailMimeHash.insert(mime);

        return true;
    }

    if (Q_LIKELY(mime == DFMGLOBAL_NAMESPACE::kMimeTypeTextPlain
                 || mimeTypeList.contains(DFMGLOBAL_NAMESPACE::kMimeTypeAppPdf)
                 || mime == DFMGLOBAL_NAMESPACE::kMimeTypeAppVRRMedia
                 || mime == DFMGLOBAL_NAMESPACE::kMimeTypeAppVMAsf
                 || mime == DFMGLOBAL_NAMESPACE::kMimeTypeAppMxf)) {
        DThumbnailProviderPrivate::hasThumbnailMimeHash.insert(mime);

        return true;
    }

    if (DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->hasThumbnail(mimeType))
        return true;

    return false;
}

QString DThumbnailProvider::thumbnailFilePath(const QFileInfo &info, Size size) const
{
    const QString &absolutePath = info.absolutePath();
    const QString &absoluteFilePath = info.absoluteFilePath();

    if (absolutePath == d->sizeToFilePath(DThumbnailProvider::Size::kSmall)
        || absolutePath == d->sizeToFilePath(DThumbnailProvider::Size::kNormal)
        || absolutePath == d->sizeToFilePath(DThumbnailProvider::Size::kLarge)
        || absolutePath == StandardPaths::location(StandardPaths::kThumbnailFailPath)) {
        return absoluteFilePath;
    }
    struct stat st;
    ulong inode = 0;
    QByteArray pathArry = absoluteFilePath.toUtf8();
    std::string pathStd = pathArry.toStdString();
    if (stat(pathStd.c_str(), &st) == 0)
        inode = st.st_ino;

    const QString thumbnailName = dataToMd5Hex((QUrl::fromLocalFile(absoluteFilePath).toString(QUrl::FullyEncoded) + QString::number(inode)).toLocal8Bit()) + kFormat;
    QString thumbnail = d->sizeToFilePath(size) + QDir::separator() + thumbnailName;
    if (!DecoratorFile(thumbnail).exists()) {
        return QString();
    }

    QImageReader ir(thumbnail, QByteArray(kFormat).mid(1));
    if (!ir.canRead()) {
        DecoratorFileOperator(thumbnail).deleteFile();
        return QString();
    }
    ir.setAutoDetectImageFormat(false);

    const QImage image = ir.read();

    if (!image.isNull() && image.text(QT_STRINGIFY(Thumb::MTime)).toInt() != (int)info.lastModified().toTime_t()) {
        DecoratorFileOperator(thumbnail).deleteFile();

        return QString();
    }

    return thumbnail;
}

static QString generalKey(const QString &key)
{
    const QStringList &tmp = key.split('/');

    if (tmp.size() > 1)
        return tmp.first() + "/*";

    return key;
}

QString DThumbnailProvider::createThumbnail(const QFileInfo &info, DThumbnailProvider::Size size)
{
    d->errorString.clear();

    const QString &DirPath = info.absolutePath();
    const QString &filePath = info.absoluteFilePath();

    if (DirPath == d->sizeToFilePath(DThumbnailProvider::Size::kSmall)
        || DirPath == d->sizeToFilePath(DThumbnailProvider::Size::kNormal)
        || DirPath == d->sizeToFilePath(DThumbnailProvider::Size::kLarge)
        || DirPath == StandardPaths::location(StandardPaths::kThumbnailFailPath)) {
        return filePath;
    }

    if (!hasThumbnail(info)) {
        d->errorString = QStringLiteral("This file has not support thumbnail: ") + filePath;

        //!Warnning: Do not store thumbnails to the fail path
        return QString();
    }

    const QString fileUrl = QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded);

    struct stat st;
    ulong inode = 0;
    QByteArray pathArray = filePath.toUtf8();
    std::string pathStd = pathArray.toStdString();
    if (stat(pathStd.c_str(), &st) == 0)
        inode = st.st_ino;
    const QString thumbnailName = dataToMd5Hex((fileUrl + QString::number(inode)).toLocal8Bit()) + kFormat;

    // the file is in fail path
    QString thumbnail = StandardPaths::location(StandardPaths::kThumbnailFailPath) + QDir::separator() + thumbnailName;

    QMimeType mime = d->mimeDatabase.mimeTypeForFile(info);
    QScopedPointer<QImage> image(new QImage());

    QStringList mimeTypeList = { mime.name() };
    mimeTypeList.append(mime.parentMimeTypes());

    //! 新增djvu格式文件缩略图预览
    if (mime.name().contains(DFMGLOBAL_NAMESPACE::kMimeTypeImageVDjvu)) {
        thumbnail = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->createThumbnail(info, (DTK_GUI_NAMESPACE::DThumbnailProvider::Size)size);
        d->errorString = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->errorString();

        if (d->errorString.isEmpty()) {
            emit createThumbnailFinished(filePath, thumbnail);

            return thumbnail;
        } else {
            QString readerBinary = QStandardPaths::findExecutable("deepin-reader");
            if (readerBinary.isEmpty())
                return thumbnail;
            //! 使用子进程来调用deepin-reader程序生成djvu格式文件缩略图
            QProcess process;
            QStringList arguments;
            //! 生成缩略图缓存地址
            QString saveImage = d->sizeToFilePath(size) + QDir::separator() + thumbnailName;
            arguments << "--thumbnail"
                      << "-f" << filePath << "-t" << saveImage;
            process.start(readerBinary, arguments);

            if (!process.waitForFinished()) {
                d->errorString = process.errorString();

                goto _return;
            }

            if (process.exitCode() != 0) {
                const QString &error = process.readAllStandardError();

                if (error.isEmpty()) {
                    d->errorString = QString("get thumbnail failed from the \"%1\" application").arg(readerBinary);
                } else {
                    d->errorString = error;
                }

                goto _return;
            }

            auto dfile = DFMBASE_NAMESPACE::DecoratorFile(saveImage).filePtr();
            if (dfile && dfile->open(DFMIO::DFile::OpenFlag::kReadOnly)) {
                QByteArray output = dfile->readAll();
                Q_ASSERT(!output.isEmpty());

                if (image->loadFromData(output, "png")) {
                    d->errorString.clear();
                }
                dfile->close();
            }
        }
    } else if (mime.name().startsWith("image/")) {
        //        mime = d->mimeDatabase.mimeTypeForFile(info, QMimeDatabase::MatchContent);
        //        QImageReader reader(absoluteFilePath, mime.preferredSuffix().toLatin1());

        //! fix bug#49451 因为使用mime.preferredSuffix(),会导致后续image.save崩溃，具体原因还需进一步跟进
        //! QImageReader构造时不传format参数，让其自行判断
        //! fix bug #53200 QImageReader构造时不传format参数，会造成没有读取不了真实的文件 类型比如将png图标后缀修改为jpg，读取的类型不对

        QString mimeType = d->mimeDatabase.mimeTypeForFile(info, QMimeDatabase::MatchContent).name();
        QString suffix = mimeType.replace("image/", "");

        QImageReader reader(filePath, suffix.toLatin1());
        if (!reader.canRead()) {
            d->errorString = reader.errorString();
            goto _return;
        }

        const QSize &imageSize = reader.size();

        //fix 读取损坏icns文件（可能任意损坏的image类文件也有此情况）在arm平台上会导致递归循环的问题
        //这里先对损坏文件（imagesize无效）做处理，不再尝试读取其image数据
        if (!imageSize.isValid()) {
            d->errorString = "Fail to read image file attribute data:" + info.absoluteFilePath();
            goto _return;
        }

        if (imageSize.width() > size || imageSize.height() > size || mime.name() == DFMGLOBAL_NAMESPACE::kMimeTypeImageSvgXml) {
            reader.setScaledSize(reader.size().scaled(size, size, Qt::KeepAspectRatio));
        }

        reader.setAutoTransform(true);

        if (!reader.read(image.data())) {
            d->errorString = reader.errorString();
            goto _return;
        }

        if (image->width() > size || image->height() > size) {
            image->operator=(image->scaled(size, size, Qt::KeepAspectRatio));
        }
    } else if (mime.name() == DFMGLOBAL_NAMESPACE::kMimeTypeTextPlain) {
        //FIXME(zccrs): This should be done using the image plugin?
        auto dfile = DFMBASE_NAMESPACE::DecoratorFile(filePath).filePtr();
        if (!dfile || !dfile->open(DFMIO::DFile::OpenFlag::kReadOnly)) {
            d->errorString = dfile->lastError().errorMsg();
            goto _return;
        }
        AbstractFileInfoPointer fileinfo = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(filePath));
        if (!fileinfo)
            goto _return;

        QString text { FileUtils::toUnicode(dfile->read(2000), fileinfo->fileName()) };

        QFont font;
        font.setPixelSize(12);

        QPen pen;
        pen.setColor(Qt::black);

        *image = QImage(0.70707070 * size, size, QImage::Format_ARGB32_Premultiplied);
        image->fill(Qt::white);

        QPainter painter(image.data());
        painter.setFont(font);
        painter.setPen(pen);

        QTextOption option;

        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        painter.drawText(image->rect(), text, option);
    } else if (mimeTypeList.contains(DFMGLOBAL_NAMESPACE::kMimeTypeAppPdf)) {
        //FIXME(zccrs): This should be done using the image plugin?
        QScopedPointer<poppler::document> doc(poppler::document::load_from_file(filePath.toStdString()));

        if (!doc || doc->is_locked()) {
            d->errorString = QStringLiteral("Cannot read this pdf file: ") + filePath;
            goto _return;
        }

        if (doc->pages() < 1) {
            d->errorString = QStringLiteral("This stream is invalid");
            goto _return;
        }

        QScopedPointer<const poppler::page> page(doc->create_page(0));

        if (!page) {
            d->errorString = QStringLiteral("Cannot get this page at index 0");
            goto _return;
        }

        poppler::page_renderer pr;
        pr.set_render_hint(poppler::page_renderer::antialiasing, true);
        pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);

        poppler::image imageData = pr.render_page(page.data(), 72, 72, -1, -1, -1, size);

        if (!imageData.is_valid()) {
            d->errorString = QStringLiteral("Render error");
            goto _return;
        }

        poppler::image::format_enum format = imageData.format();
        QImage img;

        switch (format) {
        case poppler::image::format_invalid:
            d->errorString = QStringLiteral("Image format is invalid");
            goto _return;
        case poppler::image::format_mono:
            img = QImage((uchar *)imageData.data(), imageData.width(), imageData.height(), QImage::Format_Mono);
            break;
        case poppler::image::format_rgb24:
            img = QImage((uchar *)imageData.data(), imageData.width(), imageData.height(), QImage::Format_ARGB6666_Premultiplied);
            break;
        case poppler::image::format_argb32:
            img = QImage((uchar *)imageData.data(), imageData.width(), imageData.height(), QImage::Format_ARGB32);
            break;
        default:
            break;
        }

        if (img.isNull()) {
            d->errorString = QStringLiteral("Render error");
            goto _return;
        }

        *image = img.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        thumbnail = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->createThumbnail(info, (DTK_GUI_NAMESPACE::DThumbnailProvider::Size)size);
        d->errorString = DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->errorString();

        if (d->errorString.isEmpty()) {
            emit createThumbnailFinished(filePath, thumbnail);

            return thumbnail;
        } else {   // fallback to thumbnail tool
            DFMBASE_NAMESPACE::DVideoThumbnailProvider videoProvider;

            QString mimeName = mime.name();
            bool useVideo = videoProvider.hasKey(mimeName);
            if (!useVideo) {
                mimeName = generalKey(mimeName);
                useVideo = videoProvider.hasKey(mimeName);
            }
            if (useVideo) {
                *image = videoProvider.createThumbnail(QString::number(size), filePath);
                d->errorString.clear();
            }
        }
    }

_return:
    // successful
    if (d->errorString.isEmpty()) {
        thumbnail = d->sizeToFilePath(size) + QDir::separator() + thumbnailName;
    } else {
        //fail
        image.reset(new QImage(1, 1, QImage::Format_Mono));
    }

    image->setText(QT_STRINGIFY(Thumb::URL), fileUrl);
    image->setText(QT_STRINGIFY(Thumb::MTime), QString::number(info.lastModified().toTime_t()));

    // create path
    QFileInfo(thumbnail).absoluteDir().mkpath(".");

    if (!image->save(thumbnail, Q_NULLPTR, 80)) {
        d->errorString = QStringLiteral("Can not save image to ") + thumbnail;
    }

    if (d->errorString.isEmpty()) {
        emit createThumbnailFinished(filePath, thumbnail);

        return thumbnail;
    }

    // fail
    emit createThumbnailFailed(filePath);

    return QString();
}

void DThumbnailProvider::appendToProduceQueue(const QString &filePath, DThumbnailProvider::Size size, DThumbnailProvider::CallBack callback)
{
    DThumbnailProviderPrivate::ProduceInfo produceInfo;

    produceInfo.fileInfo = QFileInfo(filePath);
    produceInfo.size = size;
    produceInfo.callback = callback;

    {
        QMutexLocker locker(&d->mutex);
        // fix bug 62540 这里在没生成缩略图的情况下，（触发刷新，文件大小改变）同一个文件会多次生成缩略图的情况,
        // 缓存当前队列中生成的缩略图文件的路劲没有就加入队里生成
        if (!d->produceAbsoluteFilePathQueue.contains(filePath)) {
            d->produceQueue.append(std::move(produceInfo));
            d->produceAbsoluteFilePathQueue.push_back(filePath);
        }
    }

    if (!isRunning())
        start();
    else
        d->waitCondition.wakeAll();
}

QString DThumbnailProvider::errorString() const
{
    return d->errorString;
}

qint64 DThumbnailProvider::sizeLimit(const QMimeType &mimeType) const
{
    return d->sizeLimitHash.value(mimeType, d->defaultSizeLimit);
}

DThumbnailProvider::DThumbnailProvider(QObject *parent)
    : QThread(parent), d(new DThumbnailProviderPrivate(this))
{
    d->init();
}

DThumbnailProvider::~DThumbnailProvider()
{
    d->running = false;
    d->waitCondition.wakeAll();
    wait();
}

void DThumbnailProvider::run()
{
    forever {

        if (d->produceQueue.isEmpty()) {
            d->waitCondition.wait(&d->mutex);
            d->mutex.unlock();
        }

        if (!d->running)
            return;

        QMutexLocker locker(&d->mutex);
        const DThumbnailProviderPrivate::ProduceInfo &task = d->produceQueue.dequeue();

        locker.unlock();
        const QString &thumbnail = createThumbnail(task.fileInfo, task.size);

        locker.relock();
        //fix 62540 生成结束了去除缓存
        d->produceAbsoluteFilePathQueue.removeOne(task.fileInfo.absoluteFilePath());
        locker.unlock();

        if (task.callback)
            task.callback(thumbnail);
    }
}
