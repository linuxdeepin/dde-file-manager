// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILFACTORY_H
#define THUMBNAILFACTORY_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QThread>

namespace dfmbase {

class ThumbnailFactoryPrivate;
class ThumbnailFactory final : public QThread
{
    Q_OBJECT
public:
    static ThumbnailFactory *instance()
    {
        static ThumbnailFactory ins;
        return &ins;
    }

    QImage createThumbnail(const QUrl &url, DFMGLOBAL_NAMESPACE::ThumbnialSize size);
    void removeTasksWithUrl(const QUrl &url);
    using ThumbnailCreator = std::function<QImage(const QString &, DFMGLOBAL_NAMESPACE::ThumbnialSize)>;
    bool registerThumbnailCreator(const QString &mimeType, ThumbnailCreator creator);

    bool contains(const QUrl &url);

Q_SIGNALS:
    void produceFinished(const QUrl &src, const QString &thumbPath);
    void produceFailed(const QUrl &src);

private Q_SLOTS:
    void onDevUnmounted(const QString &id, const QString &oldMpt);

protected:
    explicit ThumbnailFactory(QObject *parent = nullptr);
    ~ThumbnailFactory() override;
    void initConnections();
    virtual void run() override;

private:
    QScopedPointer<ThumbnailFactoryPrivate> d;
};
}   // namespace dfmbase

#endif   // THUMBNAILFACTORY_H
