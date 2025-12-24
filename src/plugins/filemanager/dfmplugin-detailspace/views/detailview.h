// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include "dfmplugin_detailspace_global.h"

#include <DFrame>
#include <QVBoxLayout>
#include <QUrl>

class QScrollArea;

namespace dfmplugin_detailspace {

class ImagePreviewWidget;
class ImagePreviewController;
class FileBaseInfoView;

class DetailView : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit DetailView(QWidget *parent = nullptr);
    virtual ~DetailView();

    bool addCustomControl(QWidget *widget);
    bool insertCustomControl(int index, QWidget *widget);
    void removeControl();

public slots:
    void setUrl(const QUrl &url, int widgetFilter);

private slots:
    void onPreviewReady(const QUrl &url, const QPixmap &pixmap);
    void onAnimatedImageReady(const QUrl &url, const QString &filePath);

private:
    void initInfoUI();
    void createHeadUI(const QUrl &url, int widgetFilter);
    void createBasicWidget(const QUrl &url, int widgetFilter);
    void updatePreviewSize();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QScrollArea *scrollArea { nullptr };
    QFrame *expandFrame { nullptr };
    QVBoxLayout *vLayout { nullptr };
    QVBoxLayout *mainLayout { nullptr };
    QList<QWidget *> expandList {};

    ImagePreviewWidget *m_previewWidget { nullptr };
    ImagePreviewController *m_previewController { nullptr };
    FileBaseInfoView *m_fileBaseInfoView { nullptr };
    QUrl m_currentUrl;
};

}

#endif   // DETAILVIEW_H
