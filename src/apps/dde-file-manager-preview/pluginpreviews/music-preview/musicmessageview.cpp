// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicmessageview.h"
#include "cover.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUrl>
#include <QResizeEvent>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QTime>
#include <QFileInfo>
#include <QBuffer>
#include <QImageReader>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/apetag.h>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QTextCodec>
#endif

using namespace plugin_filepreview;
MusicMessageView::MusicMessageView(const QString &uri, QWidget *parent)
    : QFrame(parent),
      currentUrl(uri)
{
    initUI();
    setMediaInfo();
    localeCodes.insert("zh_CN", "GB18030");
}

void MusicMessageView::initUI()
{
    setFixedSize(600, 300);

    titleLabel = new QLabel(this);
    titleLabel->setObjectName("Title");
    QFont titleFont = titleLabel->font();
    titleFont.setPixelSize(18);
    titleLabel->setFont(titleFont);
    QPalette titlePe;
    titlePe.setColor(QPalette::WindowText, QColor("#101010"));
    titleLabel->setPalette(titlePe);

    artistLabel = new QLabel(this);
    artistLabel->setObjectName("Artist");
    artistLabel->setText(tr("Artist:"));
    QFont artistLabelFont = artistLabel->font();
    artistLabelFont.setPixelSize(12);
    artistLabel->setFont(artistLabelFont);
    artistValue = new QLabel(this);
    artistValue->setObjectName("artistValue");
    QFont artistValueFont = artistValue->font();
    artistValueFont.setPixelSize(12);
    artistValue->setFont(artistValueFont);

    albumLabel = new QLabel(this);
    albumLabel->setObjectName("Album");
    albumLabel->setText(tr("Album:"));
    QFont albumLabelFont = albumLabel->font();
    albumLabelFont.setPixelSize(12);
    albumLabel->setFont(albumLabelFont);
    albumValue = new QLabel(this);
    albumValue->setObjectName("albumValue");
    QFont albumValueFont = albumValue->font();
    albumValueFont.setPixelSize(12);
    albumValue->setFont(albumValueFont);

    imgLabel = new Cover(this);
    imgLabel->setFixedSize(QSize(240, 240));

    QHBoxLayout *artistLayout = new QHBoxLayout;
    artistLayout->addWidget(artistLabel);
    artistLayout->addSpacing(5);
    artistLayout->addWidget(artistValue, 1);

    QHBoxLayout *albumLayout = new QHBoxLayout;
    albumLayout->addWidget(albumLabel);
    albumLayout->addSpacing(5);
    albumLayout->addWidget(albumValue, 1);

    QVBoxLayout *messageLayout = new QVBoxLayout;
    messageLayout->setSpacing(0);
    messageLayout->addWidget(titleLabel);
    messageLayout->addSpacing(10);
    messageLayout->addLayout(artistLayout);
    messageLayout->addSpacing(3);
    messageLayout->addLayout(albumLayout);
    messageLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(imgLabel, 0, Qt::AlignTop);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(messageLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void MusicMessageView::updateElidedText()
{
    QFontMetrics fmTitleLabel(titleLabel->font());
    titleLabel->setText(fmTitleLabel.elidedText(fileTitle, Qt::ElideRight, width() - imgLabel->width() - 40 - viewMargins));

    QFontMetrics fmArtistValue = QFontMetrics(artistValue->font());
    artistValue->setText(fmArtistValue.elidedText(fileArtist, Qt::ElideRight, width() - imgLabel->width() - 40 - viewMargins));

    QFontMetrics fmAlbumValue = QFontMetrics(albumValue->font());
    albumValue->setText(fmAlbumValue.elidedText(fileAlbum, Qt::ElideRight, width() - imgLabel->width() - 40 - viewMargins));
}

void MusicMessageView::setMediaInfo()
{
    MediaMeta meta = tagOpenMusicFile(currentUrl);
    fileTitle = meta.title;
    if (fileTitle.isEmpty()) {
        QFileInfo file(currentUrl);
        QString fileName = file.baseName();
        fileTitle = fileName;
    }
    fileArtist = meta.artist;
    if (fileArtist.isEmpty())
        fileArtist = QString(tr("unknown artist"));

    fileAlbum = meta.album;
    if (fileAlbum.isEmpty())
        fileAlbum = QString(tr("unknown album"));

    QImage img;
    QUrl url(currentUrl);
    TagLib::MPEG::File f(url.toLocalFile().toLocal8Bit());
    if (f.isValid()) {
        if (f.ID3v2Tag()) {
            TagLib::ID3v2::FrameList frameList = f.ID3v2Tag()->frameListMap()["APIC"];
            if (!frameList.isEmpty()) {
                TagLib::ID3v2::AttachedPictureFrame *picFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frameList.front());
                QBuffer buffer;
                buffer.setData(picFrame->picture().data(), static_cast<int>(picFrame->picture().size()));
                QImageReader imageReader(&buffer);
                img = imageReader.read();
            }
        }

        f.clear();
    }

    if (img.isNull()) {
        img = QImage(":/icons/icons/default_music_cover.png");
    }
    imgLabel->setCoverPixmap(QPixmap::fromImage(img).scaled(imgLabel->size(), Qt::KeepAspectRatio));

    updateElidedText();
}

void MusicMessageView::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    viewMargins = (event->size().height() - imgLabel->height()) / 2;
    if ((event->size().width() - viewMargins - 250) < imgLabel->width()) {
        viewMargins = event->size().width() - 250 - imgLabel->width();
    }
    setContentsMargins(viewMargins, viewMargins, 0, viewMargins);
    updateElidedText();
}

QList<QByteArray> MusicMessageView::detectEncodings(const QByteArray &rawData)
{
    QList<QByteArray> charsets;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QByteArray charset = QTextCodec::codecForLocale()->name();
    charsets << charset;
#else
    charsets << locale().name().toUtf8();
#endif

    const char *data = rawData.data();
    int32_t len = rawData.size();

    UCharsetDetector *csd;
    const UCharsetMatch **csm;
    int32_t matchCount = 0;

    UErrorCode status = U_ZERO_ERROR;

    csd = ucsdet_open(&status);
    if (status != U_ZERO_ERROR) {
        ucsdet_close(csd);
        return charsets;
    }

    ucsdet_setText(csd, data, len, &status);
    if (status != U_ZERO_ERROR) {
        ucsdet_close(csd);
        return charsets;
    }

    csm = ucsdet_detectAll(csd, &matchCount, &status);
    if (status != U_ZERO_ERROR) {
        ucsdet_close(csd);
        return charsets;
    }

    if (matchCount > 0) {
        charsets.clear();
    }

    for (int32_t match = 0; match < matchCount; match += 1) {
        const char *name = ucsdet_getName(csm[match], &status);
        const char *lang = ucsdet_getLanguage(csm[match], &status);
        if (lang == nullptr || strlen(lang) == 0) {
            lang = "**";
        }
        charsets << name;
    }

    ucsdet_close(csd);
    return charsets;
}

bool MusicMessageView::isChinese(const QChar &c)
{
    return c.unicode() <= 0x9FBF && c.unicode() >= 0x4E00;
}

MediaMeta MusicMessageView::tagOpenMusicFile(const QString &path)
{
    QUrl url(path);
    TagLib::FileRef f(url.toLocalFile().toLocal8Bit());

    TagLib::Tag *tag = f.tag();

    if (!f.file()) {
        fmCritical() << "Music Preview: open file failed:" << path << f.file();
    }

    if (!tag) {
        fmWarning() << "Music Preview: no tag for media file" << path;
        return MediaMeta();
    }

    MediaMeta meta;
    characterEncodingTransform(meta, static_cast<void *>(tag));
    return meta;
}

void MusicMessageView::characterEncodingTransform(MediaMeta &meta, void *obj)
{
    TagLib::Tag *tag = static_cast<TagLib::Tag *>(obj);
    bool encode = true;
    encode &= tag->title().isEmpty() ? true : tag->title().isLatin1();
    encode &= tag->artist().isEmpty() ? true : tag->artist().isLatin1();
    encode &= tag->album().isEmpty() ? true : tag->album().isLatin1();

    QByteArray detectByte;
    QByteArray detectCodec;
    if (encode) {
        if (detectCodec.isEmpty()) {
            detectByte += tag->title().toCString();
            detectByte += tag->artist().toCString();
            detectByte += tag->album().toCString();
            auto allDetectCodecs = detectEncodings(detectByte);
            auto localeCode = localeCodes.value(QLocale::system().name());

            auto iter = std::find_if(allDetectCodecs.begin(), allDetectCodecs.end(),
                                     [localeCode](const QByteArray &curDetext) {
                                         return (curDetext == "Big5" || curDetext == localeCode);
                                     });

            if (iter != allDetectCodecs.end()) {
                detectCodec = *iter;
            }

            if (detectCodec.isEmpty())
                detectCodec = allDetectCodecs.value(0);

            QString curStr = QString::fromLocal8Bit(tag->title().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->artist().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->album().toCString());

            auto ret = std::any_of(curStr.begin(), curStr.end(), [this](const QChar &ch) {
                return isChinese(ch);
            });

            if (ret) {
                detectCodec = "GB18030";
            }
        }

        QString detectCodecStr(detectCodec);
        if (detectCodecStr.compare("utf-8", Qt::CaseInsensitive) == 0) {
            meta.album = TStringToQString(tag->album());
            meta.artist = TStringToQString(tag->artist());
            meta.title = TStringToQString(tag->title());
            meta.codec = "UTF-8";   //info codec
        } else {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QTextCodec *codec = QTextCodec::codecForName(detectCodec);

            if (codec == nullptr) {
                meta.album = TStringToQString(tag->album());
                meta.artist = TStringToQString(tag->artist());
                meta.title = TStringToQString(tag->title());
            } else {
                meta.album = codec->toUnicode(tag->album().toCString());
                meta.artist = codec->toUnicode(tag->artist().toCString());
                meta.title = codec->toUnicode(tag->title().toCString());
            }
#else
            meta.album = TStringToQString(tag->album());
            meta.artist = TStringToQString(tag->artist());
            meta.title = TStringToQString(tag->title());
#endif
            meta.codec = detectCodec;
        }
    } else {
        meta.album = TStringToQString(tag->album());
        meta.artist = TStringToQString(tag->artist());
        meta.title = TStringToQString(tag->title());
        meta.codec = "UTF-8";
    }

    //empty str
    meta.album = meta.album.simplified();
    meta.artist = meta.artist.simplified();
    meta.title = meta.title.simplified();
}
