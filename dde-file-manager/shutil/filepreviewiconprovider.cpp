#include "filepreviewiconprovider.h"
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

FilePreviewIconProvider::FilePreviewIconProvider(QObject *parent) : QObject(parent)
{


}
FilePreviewIconProvider* FilePreviewIconProvider::instance(){
    static FilePreviewIconProvider* instance;
    return instance;
}
QPixmap FilePreviewIconProvider::getPlainTextPreviewIcon(QString fileUrl){
    QFile file(fileUrl);
    if(!file.exists())
        return QPixmap();

    if(!file.open(QIODevice::ReadOnly))
        return QPixmap();

    file.waitForBytesWritten(200);
    QString text = file.readAll();
    file.close();

    QStringList strs = text.split("\n");

    QImage img(QSize(PIXMAP_WIDTH,PIXMAP_HEIGHT),QImage::Format_ARGB32);
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
            if(QFontMetrics(font).width(str)<(PIXMAP_WIDTH-15))
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
QPixmap FilePreviewIconProvider::getPDFPreviewIcon(QString fileUrl){
    Poppler::Document* document = Poppler::Document::load(fileUrl);
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

    img = img.scaled(QSize(PIXMAP_WIDTH,PIXMAP_HEIGHT),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    delete pdfPage;
    return QPixmap::fromImage(img);

}

QPixmap FilePreviewIconProvider::getVideoPreviewIcon(QString fileUrl){
    AVFormatContext *pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext  *pCodecCtx = NULL;
    AVCodec         *pCodec = NULL;
    AVFrame         *pFrame = NULL;
    AVPacket        packet;
    int             frameFinished;
    int64_t         duration = 0;
    uint8_t         **frameData = NULL;
    AVFrame*        frameRGB;

    AVDictionary    *optionsDict = NULL;
    struct SwsContext *sws_ctx = NULL;

    // Register all formats and codecs for decoding vidio files
    av_register_all();

    // Open video file
    if(avformat_open_input(&pFormatCtx, fileUrl.toStdString().data(), NULL, NULL)!=0){
        qDebug()<<"couldn't open file"<<fileUrl.toStdString().data()<<fileUrl;
        return QPixmap();
    }

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0){
        qDebug()<<"couldn't fine stream infomation";
        return QPixmap();
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, fileUrl.toStdString().data(), 0);

    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
      if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
        videoStream=i;
        break;
      }
    if(videoStream==-1){
        qDebug()<<"didn't find a vidion stream";
        return QPixmap();
    }
    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;


//    AV_TIME_BASE_Q

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
//      return -1; // Codec not found
        qDebug()<<"Unsupported codec";
        return QPixmap();
    }

    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0){
        qDebug()<<"Could not open the codec";
        return QPixmap();
    }

    // Allocate video frame
    pFrame=av_frame_alloc();

    duration = pFormatCtx->duration;

    //seek frame in middle time
    if(av_seek_frame(pFormatCtx,videoStream,duration/(2*AV_TIME_BASE)*pFormatCtx->streams[videoStream]->time_base.den,AVSEEK_FLAG_BACKWARD)<0){
        qDebug()<<"Seek frame failed";
        return QPixmap();
    }

    // Read frames and save first five frames to disk
    while(av_read_frame(pFormatCtx, &packet)>=0){
        if(packet.stream_index==videoStream){
            // Decode video frame
              avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,&packet);
              if(frameFinished){
                  if(pFrame->key_frame == 1){
                      sws_ctx =
                        sws_getContext
                        (
                            pCodecCtx->width,
                            pCodecCtx->height,
                            pCodecCtx->pix_fmt,
                            pCodecCtx->width,
                            pCodecCtx->height,
                            AVPixelFormat::AV_PIX_FMT_RGB24,
                            SWS_BICUBIC,
                            NULL,
                            NULL,
                            NULL
                        );

                      frameRGB = av_frame_alloc();
                      //Allocate memory for the pixels of a picture and setup the AVPicture fields for it.
                      avpicture_alloc( ( AVPicture *) frameRGB, AV_PIX_FMT_RGB24, pFrame->width, pFrame->height);
                      qDebug()<<"Convertting image format";
                      sws_scale(sws_ctx,pFrame->data,pFrame->linesize,0,pCodecCtx->height,frameRGB->data,frameRGB->linesize);
                      qDebug()<<"Convert end";
                      frameData = frameRGB->data;
                      break;
                  }
              }
        }

    }

    if(frameData == NULL){
        qDebug()<<"frame read fialed";
        return QPixmap();
    }

    QImage img(pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB888);
    for( int y = 0; y < img.height(); ++y )
       memcpy( img.scanLine(y), frameRGB->data[0]+y * frameRGB->linesize[0], img.width() * 3 );

    img = img.scaled(QSize(PIXMAP_WIDTH,PIXMAP_HEIGHT),Qt::KeepAspectRatio,Qt::SmoothTransformation);

    av_free_packet(&packet);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    return QPixmap::fromImage(img);
}
