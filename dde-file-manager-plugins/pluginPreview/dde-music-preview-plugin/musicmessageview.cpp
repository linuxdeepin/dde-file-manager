/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "musicmessageview.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUrl>
#include <QResizeEvent>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QTime>
#include <QFileInfo>

MusicMessageView::MusicMessageView(const QString &uri, QWidget *parent) :
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
    m_artistLabel->setText(tr("Artist:"));
    m_artistValue = new QLabel(this);

    m_albumLabel = new QLabel(this);
    m_albumLabel->setObjectName("Album");
    m_albumLabel->setText(tr("Album:"));
    m_albumValue = new QLabel(this);

    m_imgLabel = new QLabel(this);
    m_imgLabel->setFixedSize(QSize(240, 240));

    m_player = new QMediaPlayer(this);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MusicMessageView::mediaStatusChanged);

    m_player->setMedia(QUrl::fromUserInput(m_uri));

    QHBoxLayout *artistLayout = new QHBoxLayout;
    artistLayout->addWidget(m_artistLabel);
    artistLayout->addSpacing(5);
    artistLayout->addWidget(m_artistValue,1);

    QHBoxLayout *albumLayout = new QHBoxLayout;
    albumLayout->addWidget(m_albumLabel);
    albumLayout->addSpacing(5);
    albumLayout->addWidget(m_albumValue,1);

    QVBoxLayout *messageLayout = new QVBoxLayout;
    messageLayout->setSpacing(0);
    messageLayout->addWidget(m_titleLabel);
    messageLayout->addSpacing(10);
    messageLayout->addLayout(artistLayout);
    messageLayout->addSpacing(3);
    messageLayout->addLayout(albumLayout);
    messageLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_imgLabel, 0, Qt::AlignTop);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(messageLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    setStyleSheet("QLabel#Title{"
                  "font-size: 18px;"
                  "font:demibold;"
                  "}"
                  "QLabel#Artist{"
                  "color: #5b5b5b;"
                  "font-size: 12px;"
                  "}"
                  "QLabel#Album{"
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
    m_artistValue->setText(fm.elidedText(m_artist, Qt::ElideRight, width() - m_imgLabel->width() - 40 - m_margins));
    m_albumValue->setText(fm.elidedText(m_album, Qt::ElideRight, width() - m_imgLabel->width() - 40 - m_margins));
}

void MusicMessageView::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia)
    {
        qDebug() << m_player->isMetaDataAvailable();
        m_title = m_player->metaData(QMediaMetaData::Title).toString();
        if(m_title.isEmpty())
        {
            QFileInfo file(m_uri);
            QString fileName = file.baseName();
            m_title = fileName;
        }
        m_artist = m_player->metaData(QMediaMetaData::AlbumArtist).toString();
        if(m_artist.isEmpty())
            m_artist = QString(tr("unknown artist"));

        m_album = m_player->metaData(QMediaMetaData::AlbumTitle).toString();
        if(m_album.isEmpty())
            m_album = QString(tr("unknown album"));

        QImage img = m_player->metaData(QMediaMetaData::CoverArtImage).value<QImage>();
        if (img.isNull()) {
            img = QImage(":/icons/icons/default_music_cover.png");
        }
        m_imgLabel->setPixmap(QPixmap::fromImage(img).scaled(m_imgLabel->size(), Qt::KeepAspectRatio));

        m_player->deleteLater();

        updateElidedText();
    }
}

void MusicMessageView::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    m_margins = (event->size().height() - m_imgLabel->height()) / 2;
    if ((event->size().width() - m_margins - 250) < m_imgLabel->width()) {
        m_margins = event->size().width() - 250 - m_imgLabel->width();
    }
    setContentsMargins(m_margins, m_margins, 0, m_margins);
    updateElidedText();
}
