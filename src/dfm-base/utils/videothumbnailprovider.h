// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOTHUMBNAILPROVIDER_H
#define VIDEOTHUMBNAILPROVIDER_H

#include <dfm-base/dfm_base_global.h>

#include <QScopedPointer>

class QImage;
class QString;

namespace dfmbase {

class VideoThumbnailProviderPrivate;
class VideoThumbnailProvider
{
public:
    VideoThumbnailProvider();
    ~VideoThumbnailProvider();

    bool hasKey(const QString &key) const;
    QImage createThumbnail(const QString &size, const QString &path);

private:
    QScopedPointer<VideoThumbnailProviderPrivate> d;
};

}

#endif   // VIDEOTHUMBNAILPROVIDER_H
