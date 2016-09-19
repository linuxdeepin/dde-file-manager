#include "thumbnailgenerator.h"

#include <QImageReader>
#include <QDebug>
#include <QPainter>
#include <QImage>
#include <QFile>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

// poppler for pdf
#include <poppler/qt5/poppler-qt5.h>

// ffmpeg
#include "libffmpegthumbnailer/videothumbnailer.h"

using namespace ffmpegthumbnailer;

ThumbnailGenerator::ThumbnailGenerator(QObject *parent) : QObject(parent)
{

}

QPixmap ThumbnailGenerator::generateThumbnail(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size)
{
    if(fileUrl.isLocalFile()){
        QString fPath = fileUrl.path();
        if(isPictureFile(fPath))
            return getPictureThumbnail(fPath, size);
        if(isTextPlainFile(fPath))
            return getTextplainThumbnail(fPath, size);
        if(isPDFFile(fPath))
            return getPDFThumbnail(fPath, size);
        if(isVideoFile(fPath))
            return getVideoThumbnail(fPath, size);
    }
    else{
        //TODO
    }

    return QPixmap();
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
    }

    return false;
}

bool ThumbnailGenerator::isTextPlainFile(const QString &fileName) const
{
    QString mimeTypeName = QMimeDatabase().mimeTypeForFile(fileName).name();
    if(mimeTypeName == "text/plain")
        return true;

    return false;
}

bool ThumbnailGenerator::isPDFFile(const QString &fileName) const
{
    QString mimeTypeName = QMimeDatabase().mimeTypeForFile(fileName).name();
    if(mimeTypeName == "application/pdf")
        return true;

    return false;
}

bool ThumbnailGenerator::isVideoFile(const QString &fileName) const
{
    QStringList extraVideoMineTypes;
    extraVideoMineTypes<<"application/vnd.adobe.flash.movie"
                      <<"application/vnd.rn-realmedia"
                      <<"application/vnd.ms-asf"
                      <<"application/mxf";

    QString mimeTypeName = QMimeDatabase().mimeTypeForFile(fileName).name();
    if(mimeTypeName.startsWith("video/") || extraVideoMineTypes.contains(mimeTypeName))
        return true;

    return false;
}

bool ThumbnailGenerator::isPictureFile(const QString &fileName) const
{
    QString mimeType = QMimeDatabase().mimeTypeForFile(fileName).name();
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
        QString mimetype = QMimeDatabase().mimeTypeForFile(fileName).name();
        set.insert("Thumb::Mimetype",mimetype);
        set.insert("Thumb::Size",QString::number(QFile(fileName).size()));
        set.insert("Thumb::URI",fileUrl.toString());
        set.insert("Thumb::MTime",QString::number(info.lastModified().toMSecsSinceEpoch()/1000));
        set.insert("Software", "Deepin File Manager");

        if(isTextPlainFile(fileName)){
            return set;
        }

        if(isPictureFile(fileName)){
            QImageReader reader(fileName);
            set.insert("Thumb::Image::Width",QString::number(reader.size().width()));
            set.insert("Thumb::Image::Height",QString::number(reader.size().height()));
            return set;
        }

        if(isPDFFile(fileName)){
            Poppler::Document* document = Poppler::Document::load(fileName);
            set.insert("Thumb::Document::Pages",QString::number(document->numPages()));
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
    Poppler::Document* document = Poppler::Document::load(fpath);
    if((!document || document->isLocked())){
        delete document;
        qDebug()<<"file reading error....";
        return QPixmap();
    }
    qDebug()<<"pdf document read successfully!";

    if(document == 0){
        qDebug()<<"pdf page is null";
        return QPixmap();
    }

    Poppler::Page* pdfPage = document->page(0); //Document start at page 0

    QImage img = pdfPage->renderToImage(72,72,0,0,pdfPage->pageSize().width(),pdfPage->pageSize().height());
    if(img.isNull()){
        qDebug()<<"render error";
        return QPixmap();
    }

    img = img.scaled(QSize(size,size),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    delete pdfPage;
    return QPixmap::fromImage(img);
}

QPixmap ThumbnailGenerator::getVideoThumbnail(const QString &fpath, const ThumbnailGenerator::ThumbnailSize &size)
{
    QString tempFile = QDir().tempPath() +"/"+QString::number(QDateTime::currentMSecsSinceEpoch())+".png";
    VideoThumbnailer vt(size,false,true,20,true);
    vt.generateThumbnail(fpath.toStdString(), Png,  tempFile.toStdString());
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

    /// ensure image size < 30MB
    if (file.size() > 1024 * 1024 * 30&&reader.canRead()) {
        return QPixmap();
    }

    QSize imgsize = reader.size();

    bool canScale = imgsize.width() > size || imgsize.height() > size;

    if (canScale) {
        imgsize.scale(QSize(qMin(256, imgsize.width()), qMin(256, imgsize.height())), Qt::KeepAspectRatio);
        reader.setScaledSize(imgsize);
    }

    return QPixmap::fromImageReader(&reader);
}

