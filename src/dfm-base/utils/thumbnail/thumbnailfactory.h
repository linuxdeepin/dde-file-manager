// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILFACTORY_H
#define THUMBNAILFACTORY_H

#include "thumbnailworker.h"

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QTimer>

namespace dfmbase {

class ThumbnailFactory final : public QObject
{
    Q_OBJECT
public:
    static ThumbnailFactory *instance()
    {
        static ThumbnailFactory ins;
        return &ins;
    }

    void joinThumbnailJob(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnailSize size);
    using ThumbnailCreator = std::function<QImage(const QString &, DFMGLOBAL_NAMESPACE::ThumbnailSize)>;
    bool registerThumbnailCreator(const QString &mimeType, ThumbnailCreator creator);

Q_SIGNALS:
    void produceFinished(const QUrl &src, const QString &thumb);
    void produceFailed(const QUrl &src);

    void addTask(const ThumbnailWorker::ThumbnailTaskMap &taskMap);
    void thumbnailJob(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnailSize size);
private Q_SLOTS:
    void onAboutToQuit();
    void pushTask();
    void doJoinThumbnailJob(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnailSize size);

protected:
    explicit ThumbnailFactory(QObject *parent = nullptr);
    ~ThumbnailFactory() override;
    void init();

private:
    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QSharedPointer<QThread> thread { nullptr };
    QSharedPointer<ThumbnailWorker> worker { nullptr };
    QTimer taskPushTimer;
};
}   // namespace dfmbase

#endif   // THUMBNAILFACTORY_H
