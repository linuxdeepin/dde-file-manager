// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILCREATORS_H
#define THUMBNAILCREATORS_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

namespace dfmbase {
namespace ThumbnailCreators {
QImage defaultThumbnailCreator(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage videoThumbnailCreator(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage videoThumbnailCreatorFfmpeg(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage videoThumbnailCreatorLib(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage textThumbnailCreator(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage audioThumbnailCreator(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage imageThumbnailCreator(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage djvuThumbnailCreator(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
QImage pdfThumbnailCreator(const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
}   // namespace ThumbnailCreators
}   // namespace dfmbase

#endif   // THUMBNAILCREATORS_H
