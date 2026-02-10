// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QWidget>
#include <QImage>
#include <QPointer>

class QLabel;

namespace plugin_filepreview {
class ImageView;
class ImagePreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT
public:
    explicit ImagePreview(QObject *parent = nullptr);
    ~ImagePreview() override;

    bool canPreview(const QUrl &url, QByteArray *format = nullptr) const;

    void initialize(QWidget *window, QWidget *statusBar) override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;

private:
    QUrl currentFileUrl;
    QPointer<QLabel> messageStatusBar;
    QPointer<ImageView> imageView;
    QString imageTitle;
};
}
#endif   // IMAGEPREVIEW_H
