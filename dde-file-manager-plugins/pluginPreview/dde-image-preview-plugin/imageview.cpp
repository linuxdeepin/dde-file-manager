#include "imageview.h"

#include <QUrl>
#include <QImageReader>
#include <QApplication>
#include <QDesktopWidget>
#include <QtMath>
#include <QPainter>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>

#define MIN_SIZE QSize(400, 300)

ImageView::ImageView(const QString &fileName, QWidget *parent)
    : QLabel(parent)
{
    setFile(fileName);
    setMinimumSize(MIN_SIZE);
    setAlignment(Qt::AlignCenter);
}

void ImageView::setFile(const QString &fileName)
{
    QImageReader reader(fileName);

    m_sourceSize = reader.size();

    if (reader.canRead()) {
        const QSize &dsize = qApp->desktop()->size();

        setPixmap(QPixmap::fromImageReader(&reader).scaled(QSize(qMin((int)(dsize.width() * 0.7), m_sourceSize.width()),
                                                                 qMin((int)(dsize.height() * 0.8), m_sourceSize.height())),
                                                           Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

QSize ImageView::sourceSize() const
{
    return m_sourceSize;
}
