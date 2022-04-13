/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "dvideothumbnailprovider.h"

#include <QImage>
#include <QProcess>
#include <QDir>
#include <QDebug>

DFMBASE_BEGIN_NAMESPACE
class DVideoThumbnailProviderPrivate
{
public:
    explicit DVideoThumbnailProviderPrivate(DVideoThumbnailProvider *qq)
        : q(qq)
    {
        videoType << "video/*";
        videoType << "application/vnd.rn-realmedia";
    }
    ~DVideoThumbnailProviderPrivate() = default;

public:
    QStringList videoType;
    DVideoThumbnailProvider *q;
};

DFMBASE_END_NAMESPACE

DFMBASE_USE_NAMESPACE

DFMBASE_NAMESPACE::DVideoThumbnailProvider::DVideoThumbnailProvider()
    : d(new DVideoThumbnailProviderPrivate(this))
{
}

DVideoThumbnailProvider::~DVideoThumbnailProvider()
{
}

bool DVideoThumbnailProvider::hasKey(const QString &key) const
{
    return d->videoType.contains(key);
}

QImage DVideoThumbnailProvider::createThumbnail(const QString &size, const QString &path)
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
            processResult = processResult.split(QRegExp("[\n]"), QString::SkipEmptyParts).last();
            if (!processResult.isEmpty()) {
                if (image.loadFromData(processResult.toLocal8Bit().data(), "png")) {
                    return image;
                }
            }
        }
    }
    return image;
}
