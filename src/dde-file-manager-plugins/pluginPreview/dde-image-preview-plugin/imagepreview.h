// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

#include <QWidget>
#include <QImage>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

class QLabel;

class ImageView;

class ImagePreview : public DFM_NAMESPACE::DFMFilePreview
{
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-image-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)
public:
    explicit ImagePreview(QObject *parent = nullptr);
    ~ImagePreview() override;

    bool canPreview(const QUrl &url, QByteArray *format = nullptr) const;

    void initialize(QWidget *window, QWidget *statusBar) override;

    bool setFileUrl(const DUrl &url) override;
    DUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;

    void copyFile() const override;

private:
    DUrl m_url;
    QPointer<QLabel> m_messageStatusBar;
    QPointer<ImageView> m_imageView;
    QString m_title;
};

#endif // IMAGEPREVIEW_H
