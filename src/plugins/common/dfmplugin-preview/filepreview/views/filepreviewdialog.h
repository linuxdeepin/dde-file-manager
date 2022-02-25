/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H
#include "dfmplugin_filepreview_global.h"
#include "dfm-base/interfaces/abstractbasepreview.h"
#include "filepreviewdialogstatusbar.h"
#include "dfm-base/utils/dialogmanager.h"

#include <DAbstractDialog>
#include <DHorizontalLine>

#include <QUrl>

DWIDGET_BEGIN_NAMESPACE
class DWindowCloseButton;
DWIDGET_END_NAMESPACE

DPFILEPREVIEW_BEGIN_NAMESPACE
class FilePreviewDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit FilePreviewDialog(const QList<QUrl> &previewUrllist, QWidget *parent = nullptr);
    ~FilePreviewDialog() override;

    void updatePreviewList(const QList<QUrl> &previewUrllist);

    void setEntryUrlList(const QList<QUrl> &entryUrlList);

    void done(int r) override;

    void DoneCurrent();

signals:
    void signalCloseEvent();

public slots:
    void playCurrentPreviewFile();

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

    QList<QUrl> fileList;
    QList<QUrl> entryUrlList;

    DTK_WIDGET_NAMESPACE::DWindowCloseButton *closeButton { nullptr };
    DTK_WIDGET_NAMESPACE::DHorizontalLine *separator { nullptr };
    FilePreviewDialogStatusBar *statusBar { nullptr };

    bool playingVideo { false };
    bool firstEnterSwitchToPage { false };
    int currentPageIndex { -1 };
    DFMBASE_NAMESPACE::AbstractBasePreview *preview { nullptr };
    DFMBASE_NAMESPACE::DialogManager *dialogManager { nullptr };
};
DPFILEPREVIEW_END_NAMESPACE
#endif
