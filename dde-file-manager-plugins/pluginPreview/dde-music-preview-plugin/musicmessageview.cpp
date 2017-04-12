#include "musicmessageview.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUrl>
#include <QResizeEvent>
#include <QMediaPlayer>
#include <QMediaMetaData>

MusicMessageView::MusicMessageView(const QString& uri, QWidget *parent) :
    QFrame(parent),
    m_uri(uri)
{
    initUI();
}

void MusicMessageView::initUI()
{
    setFixedSize(600, 300);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("Title");

    m_artistLabel = new QLabel(this);
    m_artistLabel->setObjectName("Artist");

    m_albumLabel = new QLabel(this);
    m_albumLabel->setObjectName("Albumn");

    m_imgLabel = new QLabel(this);

    QMediaPlayer* player = new QMediaPlayer;
    connect(player, &QMediaPlayer::mediaStatusChanged, [=](const QMediaPlayer::MediaStatus& status){
        if(status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia){
            m_title = player->metaData(QMediaMetaData::Title).toString();

            m_artist = player->metaData(QMediaMetaData::AlbumArtist).toString();

            m_album = player->metaData(QMediaMetaData::AlbumTitle).toString();
            QImage img = player->metaData(QMediaMetaData::CoverArtImage).value<QImage>();
            if(img.isNull()){
                img = QImage(":/icons/icons/default_music_cover.png");
            }
            m_imgLabel->setPixmap(QPixmap::fromImage(img));
            m_imgLabel->setFixedSize(img.size());
            player->deleteLater();

            setFixedSize(601, 300);
            updateElidedText();
        }
    });


    player->setMedia(QUrl::fromUserInput(m_uri));

    QVBoxLayout* messageLayout = new QVBoxLayout;
    messageLayout->setSpacing(0);
    messageLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);
    messageLayout->addSpacing(10);
    messageLayout->addWidget(m_artistLabel, 0, Qt::AlignLeft);
    messageLayout->addWidget(m_albumLabel, 0, Qt::AlignLeft);
    messageLayout->addStretch();

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_imgLabel, 0, Qt::AlignTop);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(messageLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    setStyleSheet("QLabel#Title{"
                            "font-size: 16px;"
                         "}"
                         "QLabel#Artist{"
                            "color: #5b5b5b;"
                            "font-size: 12px;"
                         "}"
                         "QLabel#Albumn{"
                            "color: #5b5b5b;"
                            "font-size: 12px;"
                         "}");

}

void MusicMessageView::updateElidedText()
{   
    QFont font;
    font.setPixelSize(16);
    QFontMetrics fm(font);
    m_titleLabel->setText(fm.elidedText(m_title, Qt::ElideRight, width() - m_imgLabel->width() - 40 - m_margins));

    font.setPixelSize(12);
    fm = QFontMetrics(font);
    m_artistLabel->setText(fm.elidedText(m_artist, Qt::ElideRight, width() - m_imgLabel->width() - 40 - m_margins));
    m_albumLabel->setText(fm.elidedText(m_album, Qt::ElideRight, width() - m_imgLabel->width() - 40 - m_margins));

}

void MusicMessageView::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    m_margins = (event->size().height() - m_imgLabel->height()) / 2;
    if((event->size().width() - m_margins - 250) < m_imgLabel->width()){
        m_margins = event->size().width() - 250 - m_imgLabel->width();
    }
    setContentsMargins(m_margins, m_margins, 0, m_margins);
    updateElidedText();
}
