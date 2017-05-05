#include "videopreviewplugin.h"

#include <QVideoWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTime>
#include <QTimer>
#include <QtMath>

#include "dvideowidget.h"
//#include "mpvproxywidget.h"

VideoPreviewPlugin::VideoPreviewPlugin(QObject *parent) :
    PreviewInterface(parent)
{
    m_supportyMimeTypes << "application/vnd.adobe.flash.movie"
                        << "application/vnd.rn-realmedia"
                        << "application/vnd.ms-asf"
                        << "application/mxf";
}

void VideoPreviewPlugin::init(const QString &uri)
{
    m_uri = uri;

    QTimer* playTimer = new QTimer(this);
    playTimer->setInterval(1000);

    m_videoWidget = new DVideoWidget;

//    m_mpvProxyWidget = new MpvProxyWidget;

    m_toolBarFrame = new QFrame;
    m_progressSlider = new QSlider(m_toolBarFrame);
    m_progressSlider->setOrientation(Qt::Horizontal);
    m_durationLabel = new QLabel(m_toolBarFrame);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_progressSlider, 0, Qt::AlignVCenter);
    layout->addWidget(m_durationLabel, 0, Qt::AlignVCenter);
    m_toolBarFrame->setLayout(layout);

//    connect(m_mpvProxyWidget, &MpvProxyWidget::fileLoaded, [=]{
//        qint64 d = m_mpvProxyWidget->duration();
//        int h = d / 60 / 60;
//        int m = d % (60 * 60) / 60;
//        int s = d % 60;

//        QTime t(h, m, s);
//        m_durationLabel->setText(t.toString("HH:mm:ss"));

//        m_progressSlider->setMinimum(0);
//        m_progressSlider->setMaximum(d);
//    });

    connect(m_videoWidget, &DVideoWidget::fileLoaded, [=]{
        qint64 d = m_videoWidget->duration();
        int h = d / 60 / 60;
        int m = d % (60 * 60) / 60;
        int s = d % 60;

        QTime t(h, m, s);
        m_durationLabel->setText(t.toString("HH:mm:ss"));

        m_progressSlider->setMinimum(0);
        m_progressSlider->setMaximum(d);
    });

    connect(m_videoWidget, &DVideoWidget::stateChanged, [=] (const DVideoWidget::State& state){
        switch (state) {
        case DVideoWidget::Playing:{
            if(playTimer){
                playTimer->start();
            }
            break;
        }
        case DVideoWidget::Paused:{
            if(playTimer){
                playTimer->stop();
            }
            break;
        }
        default:
            break;
        }
    });

    connect(m_progressSlider, &QSlider::valueChanged, [=](const int& value){
        if(qAbs(value - m_videoWidget->pos()) > 10){
            m_videoWidget->seekPos(value);
        }
    });

    connect(m_videoWidget, &DVideoWidget::destroyed, [=]{
        playTimer->stop();
        playTimer->deleteLater();
    });

    connect(playTimer, &QTimer::timeout, [=]{
        m_progressSlider->setValue(m_videoWidget->pos());
    });

    m_videoWidget->loadFile(uri);

//    m_mpvProxyWidget->loadSource(uri);
}

QWidget *VideoPreviewPlugin::previewWidget()
{
    return m_videoWidget;
//    return m_mpvProxyWidget;
}

QSize VideoPreviewPlugin::previewWidgetMinSize() const
{
    return QSize(600, 400);
}

bool VideoPreviewPlugin::canPreview() const
{
    QMimeDatabase md;
    QString mimeTypeName = md.mimeTypeForFile(QUrl(m_uri).path()).name();
    if(mimeTypeName.startsWith("video/") || m_supportyMimeTypes.contains(mimeTypeName)){
        return true;
    }

    return false;
}

QWidget *VideoPreviewPlugin::toolBarItem()
{
    return m_toolBarFrame;
}

QString VideoPreviewPlugin::pluginName() const
{
    return "dde-video-preview-plugin";
}

QIcon VideoPreviewPlugin::pluginLogo() const
{
    return QIcon();
}

QString VideoPreviewPlugin::pluginDescription() const
{
    return "Deepin video preview plugin";
}

QString VideoPreviewPlugin::formatNumberString(const int &number) const
{
    if(number < 10){
        return "0" + QString::number(number);
    } else {
        return QString::number(number);
    }
}
