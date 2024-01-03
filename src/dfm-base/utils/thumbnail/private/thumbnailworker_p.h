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
#include <QTimer>

namespace dfmbase {

class ThumbnailWorkerPrivate
{
public:
    explicit ThumbnailWorkerPrivate(ThumbnailWorker *qq);
    QString createThumbnail(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnailSize size);
    bool checkFileStable(const QUrl &url);
    void startDelayWork();

    QUrl setCheckCount(const QUrl &url, int count);
    int checkCount(const QUrl &url);
    QUrl clearCheckCount(const QUrl &url);

    ThumbnailWorker *q { nullptr };
    DMimeDatabase mimeDb;
    QMap<QString, ThumbnailWorker::ThumbnailCreator> creators;
    QUrl originalUrl;
    ThumbnailHelper thumbHelper;
    std::atomic_bool isStoped = false;
    QTimer *delayTimer { nullptr };
    ThumbnailWorker::ThumbnailTaskMap delayTaskMap;
};

}   // namespace dfmbase

#endif   // THUMBNAILWORKER_P_H
