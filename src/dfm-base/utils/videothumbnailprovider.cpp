// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videothumbnailprovider.h"

#include <QImage>
#include <QProcess>
#include <QDir>
#include <QDebug>

namespace dfmbase {
class VideoThumbnailProviderPrivate
{
public:
    explicit VideoThumbnailProviderPrivate(VideoThumbnailProvider *qq)
        : q(qq)
    {
        videoType << "video/*";
        videoType << "application/vnd.rn-realmedia";
    }
    ~VideoThumbnailProviderPrivate() = default;

public:
    QStringList videoType;
    VideoThumbnailProvider *q;
};

}

using namespace dfmbase;

DFMBASE_NAMESPACE::VideoThumbnailProvider::VideoThumbnailProvider()
    : d(new VideoThumbnailProviderPrivate(this))
{
}

VideoThumbnailProvider::~VideoThumbnailProvider()
{
}

bool VideoThumbnailProvider::hasKey(const QString &key) const
{
    return d->videoType.contains(key);
}

QImage VideoThumbnailProvider::createThumbnail(const QString &size, const QString &path)
{
    QImage image;
    QByteArray output;
    try {
        QProcess ffmpeg;
        QStringList cmds { "-nostats", "-loglevel", "0",
                           "-i", QDir::toNativeSeparators(path),
                           "-vf", "scale='min(" + size + ",iw)':-1",
                           "-f", "image2pipe", "-vcodec", "png",
                           "-fs", "9000", "-" };

        ffmpeg.start("ffmpeg", cmds, QIODevice::ReadOnly);
        if (!ffmpeg.waitForFinished()) {
            qWarning() << ffmpeg.errorString();
            return QImage();
        }

        output = ffmpeg.readAllStandardOutput();
    } catch (std::logic_error &e) {
        qWarning() << e.what();
        return QImage();
    }

    if (!output.isNull()) {
        if (image.loadFromData(output, "png")) {
            return image;
        } else {
            //过滤video tool的其他输出信息
            QString processResult(output);
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
            processResult = processResult.split(QRegExp("[\n]"), QString::SkipEmptyParts).last();
#else
            processResult = processResult.split(QRegExp("[\n]"), Qt::SkipEmptyParts).last();
#endif
            if (!processResult.isEmpty()) {
                if (image.loadFromData(processResult.toLocal8Bit().data(), "png")) {
                    return image;
                }
            }
        }
    }
    return image;
}
