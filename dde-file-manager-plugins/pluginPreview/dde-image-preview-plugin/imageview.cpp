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

ImageView::ImageView(const QString &file, QWidget *parent) : QFrame(parent)
{
    QImageReader reader(QUrl(file).path());
    if(reader.canRead()){
        m_img = reader.read();

        QSize dsize = qApp->desktop()->size();

        m_img = m_img.scaled(QSize(qMin((int)(dsize.width() * 0.7), m_img.width()), qMin((int)(dsize.height() * 0.8), m_img.height())),
                             Qt::KeepAspectRatio, Qt::SmoothTransformation);

        setFixedSize(QSize(qMax(m_img.width(), MIN_SIZE.width()), qMax(m_img.height(), MIN_SIZE.height())));
    }

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(new QLabel(this));
    hlayout->addStretch(0);
    hlayout->addWidget(new QLabel(this));
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addStretch(0);
    layout->addLayout(hlayout);
    setLayout(layout);
}

void ImageView::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    QImage img  = m_img;
    int dx = (width() - img.width())/2;
    int dy = (height() - img.height())/2;

    if(dx < 0 || dy < 0){
        img = img.scaled(qMin(width(), img.width()), qMin(height(), img.height()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        dx = (width() - img.width())/2;
        dy = (height() - img.height())/2;
    }

    if(!m_img.isNull()){
        QPainter painter(this);
        painter.drawImage(dx, dy, img);
    }
}
