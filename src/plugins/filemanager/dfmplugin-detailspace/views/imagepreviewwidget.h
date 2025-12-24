// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

#include "dfmplugin_detailspace_global.h"

#include <QWidget>
#include <QPixmap>

namespace dfmplugin_detailspace {

inline constexpr int kPreviewMargin = 10;
inline constexpr qreal kPreviewAspectRatio = 1.3;

class ImagePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    QPixmap pixmap() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_pixmap;
};

}

#endif   // IMAGEPREVIEWWIDGET_H
