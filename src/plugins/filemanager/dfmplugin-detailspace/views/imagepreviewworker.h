// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEWWORKER_H
#define IMAGEPREVIEWWORKER_H

#include "dfmplugin_detailspace_global.h"

#include <QObject>
#include <QThread>
#include <QUrl>
#include <QPixmap>
#include <QAtomicInt>

namespace dfmplugin_detailspace {

class ImagePreviewWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImagePreviewWorker(QObject *parent = nullptr);
    ~ImagePreviewWorker() override;

public Q_SLOTS:
    void loadPreview(const QUrl &url, const QSize &targetSize);
    void stop();

Q_SIGNALS:
    void previewReady(const QUrl &url, const QPixmap &pixmap);
    void loadFailed(const QUrl &url);
    void needIconFallback(const QUrl &url, const QSize &targetSize);

private:
    bool isImageMimeType(const QString &mimeType) const;
    QPixmap loadOriginalImage(const QString &filePath, const QSize &targetSize);
    QPixmap loadThumbnail(const QUrl &url, const QSize &targetSize);

private:
    QAtomicInt m_stopped { false };
};

class ImagePreviewController : public QObject
{
    Q_OBJECT
public:
    explicit ImagePreviewController(QObject *parent = nullptr);
    ~ImagePreviewController() override;

    void requestPreview(const QUrl &url, const QSize &targetSize);

Q_SIGNALS:
    void previewReady(const QUrl &url, const QPixmap &pixmap);
    void loadFailed(const QUrl &url);
    void doLoadPreview(const QUrl &url, const QSize &targetSize);

private Q_SLOTS:
    void onNeedIconFallback(const QUrl &url, const QSize &targetSize);

private:
    QThread m_workerThread;
    ImagePreviewWorker *m_worker { nullptr };
};

}

#endif   // IMAGEPREVIEWWORKER_H
