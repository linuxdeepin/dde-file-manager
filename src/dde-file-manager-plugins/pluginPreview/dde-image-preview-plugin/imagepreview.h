/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
