// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include "dfmplugin_detailspace_global.h"

#include <DFrame>
#include <DSpinner>
#include <QVBoxLayout>
#include <QUrl>
#include <QTimer>

class QScrollArea;

namespace dfmplugin_detailspace {

class ImagePreviewWidget;
class ImagePreviewController;
class FileInfoWidget;

// Extension widget holder for reusable pattern
struct ExtensionWidgetHolder
{
    QWidget *widget { nullptr };
    QFrame *frame { nullptr };
    ViewExtensionUpdateFunc update;
    ViewExtensionShouldShowFunc shouldShow;
};

class DetailView : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit DetailView(QWidget *parent = nullptr);
    virtual ~DetailView();

public slots:
    void setUrl(const QUrl &url);

private slots:
    void onPreviewReady(const QUrl &url, const QPixmap &pixmap);
    void onAnimatedImageReady(const QUrl &url, const QString &filePath);

private:
    void initInfoUI();
    void initSpinnerOverlay();
    void createExtensionWidgets();
    void updateHeadUI(const QUrl &url);
    void updateBasicWidget(const QUrl &url);
    void updateExtensionWidgets(const QUrl &url);
    void updatePreviewSize();
    void reloadPreviewFile();
    void startPreviewLoading();
    void finishPreviewLoading();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QScrollArea *scrollArea { nullptr };
    QFrame *expandFrame { nullptr };
    QVBoxLayout *vLayout { nullptr };
    QVBoxLayout *mainLayout { nullptr };

    ImagePreviewWidget *m_previewWidget { nullptr };
    ImagePreviewController *m_previewController { nullptr };
    FileInfoWidget *m_fileInfoWidget { nullptr };

    // Reusable extension widgets
    QList<ExtensionWidgetHolder> m_extensionWidgets;

    // Loading state management
    QWidget *m_spinnerOverlay { nullptr };
    DTK_WIDGET_NAMESPACE::DSpinner *m_spinner { nullptr };
    QTimer *m_loadingTimer { nullptr };
    static constexpr int kSpinnerDelayMs = 200;

    QUrl m_currentUrl;
};

}

#endif   // DETAILVIEW_H
