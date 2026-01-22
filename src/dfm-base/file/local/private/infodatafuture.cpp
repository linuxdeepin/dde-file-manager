// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "infodatafuture.h"
#include "utils/fileinfohelper.h"

#include <QImageReader>

USING_IO_NAMESPACE
namespace dfmbase {
InfoDataFuture::InfoDataFuture(DFileFuture *future, QObject *parent)
    : QObject(parent), future(future)
{
    if (future) {
        qRegisterMetaType<QMap<DFileInfo::AttributeExtendID, QVariant>>("QMap<DFileInfo::AttributeExtendID, QVariant>");
        connect(future, &DFileFuture::infoMedia, this, &InfoDataFuture::infoMedia);
        connect(this, &InfoDataFuture::infoMediaAttributes, &FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished);
    }
}

QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> InfoDataFuture::mediaInfo() const
{
    return attribute;
}

bool InfoDataFuture::isFinished() const
{
    return finshed;
}

InfoDataFuture::~InfoDataFuture()
{
}

void InfoDataFuture::infoMedia(const QUrl &url, const QMap<DFileInfo::AttributeExtendID, QVariant> &map)
{
    // 先处理数据缓存下来，再转发信号给infohelper，析构future
    attribute = std::move(map);
    finshed = true;

    int width = attribute[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
    int height = attribute[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();

    // not all formats of image files have width and height in properties
    // if properties failed, use QImageReader as fallback for local files
    if ((width <= 0 || height <= 0) && url.isLocalFile()) {
        QImageReader reader(url.toLocalFile());
        if (reader.canRead()) {
            const QSize size = reader.size();
            if (size.isValid() && size.width() > 0 && size.height() > 0) {
                width = size.width();
                height = size.height();
                attribute[DFileInfo::AttributeExtendID::kExtendMediaWidth] = width;
                attribute[DFileInfo::AttributeExtendID::kExtendMediaHeight] = height;
            }
        }
    }

    emit infoMediaAttributes(url, attribute);
    future.reset(nullptr);
}

}
