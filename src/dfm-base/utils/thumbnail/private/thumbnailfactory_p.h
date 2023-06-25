// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILFACTORY_P_H
#define THUMBNAILFACTORY_P_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>
#include <dfm-base/mimetype/dmimedatabase.h>

namespace dfmbase {

struct ProduceTask
{
    QUrl srcUrl;
    DFMGLOBAL_NAMESPACE::ThumbnialSize size;
};

class ThumbnailFactoryPrivate
{
public:
    explicit ThumbnailFactoryPrivate(ThumbnailFactory *qq);
    QString createThumbnail(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
    bool checkFileStable(const QUrl &url);

    ThumbnailFactory *q { nullptr };
    DMimeDatabase mimeDb;
    QMutex mutex;
    QWaitCondition waitCon;

    bool running { false };

    QQueue<ProduceTask> produceTasks;
    QMap<QString, ThumbnailFactory::ThumbnailCreator> creators;
};

}   // namespace dfmbase

#endif   // THUMBNAILFACTORY_P_H
