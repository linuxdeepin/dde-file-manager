#include "thumbnailgenerator.h"

#include <QImageReader>
#include <QDebug>
#include <QPainter>
#include <QImage>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QPainter>
#include <QDateTime>
#include <QThread>

// poppler for pdf
//#include <poppler/qt5/poppler-qt5.h>

// use original poppler api
#include <poppler-document.h>
#include <poppler-image.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>

// ffmpeg
#include "libffmpegthumbnailer/videothumbnailer.h"

#include "dutility.h"

DWIDGET_USE_NAMESPACE

using namespace ffmpegthumbnailer;

ThumbnailGenerator::ThumbnailGenerator(QObject *parent) : QObject(parent)
{

}

QPixmap ThumbnailGenerator::generateThumbnail(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size)
{
    QPixmap pixmap;

    if(fileUrl.isLocalFile()){
        QString fPath = fileUrl.path();
        if(isPictureFile(fPath))
            pixmap = getPictureThumbnail(fPath, size);
        if(isTextPlainFile(fPath))
            pixmap = getTextplainThumbnail(fPath, size);
        if(isPDFFile(fPath))
            pixmap = getPDFThumbnail(fPath, size);
        if(isVideoFile(fPath))
            pixmap = getVideoThumbnail(fPath, size);
    }
    else{
        //TODO
    }

    return pixmap;
}

bool ThumbnailGenerator::canGenerateThumbnail(const QUrl&  fileUrl) const
{
    if(fileUrl.isLocalFile()){
        QString fPath = fileUrl.path();

        if(isPictureFile(fPath))
            return true;
        if(isTextPlainFile(fPath))
            return true;
        if(isPDFFile(fPath))
            return true;
        if(isVideoFile(fPath))
            return true;
    }
    else{
        //TODO
        return false;
    }

    return false;
}

bool ThumbnailGenerator::isVideoFile(const QString &fileName)
{
    QStringList extraVideoMineTypes;
    extraVideoMineTypes<<"application/vnd.adobe.flash.movie"
                      <<"application/vnd.rn-realmedia"
                      <<"application/vnd.ms-asf"
                      <<"application/mxf";

    QString mimeTypeName = DMimeDatabase().mimeTypeForFile(fileName).name();
    if(mimeTypeName.startsWith("video/") || extraVideoMineTypes.contains(mimeTypeName))
        return true;

    return false;
}

bool ThumbnailGenerator::isPictureFile(const QString &fileName)
{
    QString mimeType = DMimeDatabase().mimeTypeForFile(fileName).name();

    if(mimeType.startsWith("image"))
        return true;
    else
        return false;
}

QMap<QString,QString> ThumbnailGenerator::getAttributeSet(const QUrl&  fileUrl)
{
    QMap<QString,QString> set;
    if(fileUrl.isLocalFile()){
        QString fileName = fileUrl.path();
        QFileInfo info(fileName);
        QString mimetype = DMimeDatabase().mimeTypeForFile(fileName).name();
        set.insert("Thumb::Mimetype",mimetype);
        set.insert("Thumb::Size",QString::number(info.size()));
        set.insert("Thumb::URI",fileUrl.toString());
        set.insert("Thumb::MTime",QString::number(info.lastModified().toMSecsSinceEpoch()/1000));
        set.insert("Software", "Deepin File Manager");

        if(isTextPlainFile(fileName)){
            return set;
        }

        if(isPictureFile(fileName)){
            QImageReader reader(fileName);
            if(reader.canRead()){
                set.insert("Thumb::Image::Width",QString::number(reader.size().width()));
                set.insert("Thumb::Image::Height",QString::number(reader.size().height()));
            }
            return set;
        }

        if(isPDFFile(fileName)){
            // handle fail thumbnailing files
            auto doc = poppler::document::load_from_file(fileName.toStdString());
            if((!doc || doc->is_locked())){
                delete doc;
                return set;
            }

            set.insert("Thumb::Document::Pages",QString::number(doc->pages()));
            delete doc;
            return set;
        }

        else if(isVideoFile(fileName)){
            //TODO
            return set;
        }
    }
    else{
        //TODO for other's scheme
    }

    return set;
}

QPixmap ThumbnailGenerator::getTextplainThumbnail(const QString &fpath, const ThumbnailGenerator::ThumbnailSize &size)
{
    QFile file(fpath);
    if(!file.exists())
        return QPixmap();

    if(!file.open(QIODevice::ReadOnly))
        return QPixmap();

    file.waitForBytesWritten(200);
    QString text = file.readAll();
    file.close();

    QStringList strs = text.split("\n");

    QImage img(QSize(size,size),QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    QFont font;
    font.setPixelSize(12);
    painter.setFont(font);
    QPen pen;
    pen.setColor(QColor(0,0,0));
    painter.setPen(pen);

    int counter = 0;
    for(auto line : strs){
        QString str;
        QStringList subStrs;
        for(int i = 0;i<line.length();i++){
            if(QFontMetrics(font).width(str)<(size-15))
                str += line[i];
            else{
                subStrs<<str;
                str ="";
            }
        }

        for(auto subline:subStrs){
            painter.drawText(5,12+counter*15,subline);
            counter ++;
        }

        painter.drawText(5,12+counter*15,str);
        counter ++;
    }

    return QPixmap::fromImage(img);
}

QPixmap ThumbnailGenerator::getPDFThumbnail(const QString &fpath, const ThumbnailGenerator::ThumbnailSize &size)
{
    poppler::document *doc = poppler::document::load_from_file(fpath.toStdString());
    if(!doc || doc->is_locked()){
        qDebug () << "Cannot read this pdf file:" << fpath;
        return QPixmap();
    }
    if(doc->pages()<1){
        qDebug () << "This stream is invalid";
        return QPixmap();
    }

    const poppler::page *page = doc->create_page(0);

    if(!page){
        qDebug () << "Cannot get this page at index 0";
        return QPixmap();
    }

    poppler::page_renderer pr;
    pr.set_render_hint(poppler::page_renderer::antialiasing, true);
    pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);

    poppler::image imageData = pr.render_page(page);

    if(!imageData.is_valid()){
        qDebug () << "Render error";
        return QPixmap();
    }

    poppler::image::format_enum format = imageData.format();
    QImage img;

    switch (format) {
    case poppler::image::format_invalid:
        qDebug () << "Image format is invalid";
        break;
    case poppler::image::format_mono:
        qDebug () << "Image format is mono";
        img = QImage((uchar*)imageData.data(),imageData.width(),imageData.height(),QImage::Format_Mono);
        break;
    case poppler::image::format_rgb24:
        qDebug () << "Image format is argb24";
        img = QImage((uchar*)imageData.data(),imageData.width(),imageData.height(),QImage::Format_ARGB6666_Premultiplied);
        break;
    case poppler::image::format_argb32:
        qDebug () << "Image format is argb32";
        img = QImage((uchar*)imageData.data(),imageData.width(),imageData.height(),QImage::Format_ARGB32);
        break;
    default:
        break;
    }

    if(img.isNull()){
        qDebug()<<"render error";
        return QPixmap();
    }

    img = img.scaled(QSize(size,size),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    return QPixmap::fromImage(img);
}

QPixmap ThumbnailGenerator::getVideoThumbnail(const QString &fpath, const ThumbnailGenerator::ThumbnailSize &size)
{
    QString tempFile = QDir().tempPath() +"/"+QString::number(QDateTime::currentMSecsSinceEpoch())+".png";

    try{
        VideoThumbnailer vt(size,false,true,20,false);
        vt.generateThumbnail(fpath.toStdString(), Png,  tempFile.toStdString());
    }
    catch(std::logic_error e){
        qDebug()<<e.what();
        return QPixmap();
    }

    QPixmap pixmap = QPixmap::fromImage(QImage(tempFile));
    QFile file(tempFile);

    if(file.exists())
        file.remove();
    return pixmap;
}

QPixmap ThumbnailGenerator::getPictureThumbnail(const QString &fpath, const ThumbnailGenerator::ThumbnailSize &size)
{
    QFile file(fpath);
    QImageReader reader(&file);

    if(!reader.canRead())
        return QPixmap();

    /// ensure image size < 30MB
    if (file.size() > 1024 * 1024 * 30&&reader.canRead()) {
        return QPixmap();
    }

    QSize imgsize = reader.size();
    if(!imgsize.isValid()){
        qDebug () << "Cannot reader this image:" <<fpath;
        return QPixmap();
    }

    bool canScale = imgsize.width() > size || imgsize.height() > size;

    if (canScale) {
        imgsize.scale(QSize(qMin((int)size, imgsize.width()), qMin((int)size, imgsize.height())), Qt::KeepAspectRatio);
        reader.setScaledSize(imgsize);
    }

    return QPixmap::fromImageReader(&reader);
}

