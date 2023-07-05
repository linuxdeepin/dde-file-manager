// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILFACTORY_H
#define THUMBNAILFACTORY_H

#include "thumbnailworker.h"

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

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

    QImage thumbnailImage(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnailSize size);
    using ThumbnailCreator = std::function<QImage(const QString &, DFMGLOBAL_NAMESPACE::ThumbnailSize)>;
    bool registerThumbnailCreator(const QString &mimeType, ThumbnailCreator creator);

Q_SIGNALS:
    void produceFinished(const QUrl &src, const QString &thumbPath);
    void produceFailed(const QUrl &src);

    void addTask(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnailSize size);
    void removeTask(const QUrl &url);

private Q_SLOTS:
    void onDevUnmounted(const QString &id, const QString &oldMpt);
    void aboutToQuit();

protected:
    explicit ThumbnailFactory(QObject *parent = nullptr);
    ~ThumbnailFactory() override;
    void init();

private:
    QSharedPointer<QThread> thread { nullptr };
    QSharedPointer<ThumbnailWorker> worker { nullptr };
};
}   // namespace dfmbase

#endif   // THUMBNAILFACTORY_H
