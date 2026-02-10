// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H
#include "dfmplugin_filepreview_global.h"
#include <dfm-base/interfaces/abstractbasepreview.h>
#include "filepreviewdialogstatusbar.h"
#include <dfm-base/utils/dialogmanager.h>

#include <DAbstractDialog>
#include <DHorizontalLine>
#include <DFloatingButton>

#include <QUrl>

namespace dfmplugin_filepreview {
class FilePreviewDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit FilePreviewDialog(const QList<QUrl> &previewUrllist, QWidget *parent = nullptr);
    ~FilePreviewDialog() override;

    void updatePreviewList(const QList<QUrl> &previewUrllist);

    void setEntryUrlList(const QList<QUrl> &entryUrlList);

    void done(int r) override;

    void setCurrentWinID(quint64 winID);

signals:
    void signalCloseEvent();

public slots:
    void playCurrentPreviewFile();

    void openFile();

    void handleFileInfoRefreshFinished(const QUrl url, const QString &infoPtr, const bool isLinkOrg);

private:
    void childEvent(QChildEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void initUI();
    void switchToPage(int index);
    void previousPage();
    void nextPage();
    void updateTitle();
    QString generalKey(const QString &key);

    // 保存/恢复中心位置，保证切换视图后窗口仍居中到之前的位置
    void saveCenterPos();
    void restoreCenterPos();

    QList<QUrl> fileList;

    DTK_WIDGET_NAMESPACE::DFloatingButton *closeBtn { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DHorizontalLine *separator { nullptr };
    FilePreviewDialogStatusBar *statusBar { nullptr };

    bool playingVideo { false };
    bool firstEnterSwitchToPage { false };
    bool previewDir { false };
    int currentPageIndex { -1 };
    quint64 currentWinID { 0 };
    DFMBASE_NAMESPACE::AbstractBasePreview *preview { nullptr };
    DFMBASE_NAMESPACE::DialogManager *dialogManager { nullptr };
    // 在切换视图前记录中心点, 切换后恢复
    QPointF previousCenter { 0.0, 0.0 };
};
}
#endif
