// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILWORKER_P_H
#define THUMBNAILWORKER_P_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/thumbnail/thumbnailworker.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>
#include <dfm-base/mimetype/dmimedatabase.h>

#include <QFuture>

namespace dfmbase {

struct ProduceTask
{
    QUrl srcUrl;
    DFMGLOBAL_NAMESPACE::ThumbnailSize size;
};

class ThumbnailWorkerPrivate
{
public:
    explicit ThumbnailWorkerPrivate(ThumbnailWorker *qq);
    QString createThumbnail(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnailSize size);
    bool checkFileStable(const QUrl &url);

    ThumbnailWorker *q { nullptr };
    DMimeDatabase mimeDb;
    QMutex mutex;
    QWaitCondition waitCon;

    std::atomic_bool isRunning { false };

    QFuture<void> future;
    QQueue<ProduceTask> produceTasks;
    QMap<QString, ThumbnailWorker::ThumbnailCreator> creators;
};

}   // namespace dfmbase

#endif   // THUMBNAILWORKER_P_H
