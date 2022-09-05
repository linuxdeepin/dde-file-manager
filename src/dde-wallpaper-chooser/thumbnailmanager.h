// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>
#include <QQueue>
#include <QFutureWatcher>
#include <QPixmap>

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailManager(qreal scale);
    ~ThumbnailManager();

    void clear();
    void find(const QString & key);
    void remove(const QString & key);
    bool replace(const QString & key, const QPixmap & pixmap);

    void stop();
    qreal scale() const;

    static ThumbnailManager * instance(qreal scale);

signals:
    void thumbnailFounded(const QString &key, const QPixmap &pixmap);
    void findAborted(QQueue<QString> queue);

private:
    void processNextReq();

private slots:
    void onProcessFinished();

private:
    QQueue<QString> m_queuedRequests;
    QString m_cacheDir;
    QFutureWatcher<QPixmap> m_futureWatcher;
    qreal m_scale;
};

#endif // THUMBNAILMANAGER_H
