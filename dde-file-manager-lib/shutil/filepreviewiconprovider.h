#ifndef FILEPREVIEWICONPROVIDER_H
#define FILEPREVIEWICONPROVIDER_H

#include <QObject>
#include <QFile>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QDebug>
#include <poppler/qt5/poppler-qt5.h>

//#ifndef INT64_C
//#define INT64_C(c) (c ## LL)
//#define UINT64_C(c) (c ## ULL)
//#endif


#define PIXMAP_WIDTH 256
#define PIXMAP_HEIGHT 256


class FilePreviewIconProvider : public QObject
{
    Q_OBJECT
public:
    static FilePreviewIconProvider* instance();

    static QPixmap getPlainTextPreviewIcon(QString fileUrl);
    static QPixmap getPDFPreviewIcon(QString fileUrl);
    static QPixmap getVideoPreviewIcon(QString fileUrl);

signals:

public slots:

private:
    explicit FilePreviewIconProvider(QObject *parent = 0);
};

#endif // FILEPREVIEWICONPROVIDER_H
