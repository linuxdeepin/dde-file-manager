// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

#include "dfmplugin_detailspace_global.h"

#include <QWidget>
#include <QPixmap>

class QMovie;

namespace dfmplugin_detailspace {

inline constexpr int kPreviewMargin = 10;
inline constexpr qreal kPreviewAspectRatio = 1.3;

class ImagePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget *parent = nullptr);
    ~ImagePreviewWidget() override;

    void setPixmap(const QPixmap &pixmap);
    QPixmap pixmap() const;

    void setAnimatedImage(const QString &filePath);
    void stopAnimatedImage();

    QSize sizeHint() const override;

    static bool isAnimatedMimeType(const QString &mimeType);
    static QSize maximumPreviewSize();

protected:
    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void onMovieFrameChanged();

private:
    QPixmap m_pixmap;
    QMovie *m_movie { nullptr };
    bool m_hasAnimatedImage { false };
};

}

#endif   // IMAGEPREVIEWWIDGET_H
