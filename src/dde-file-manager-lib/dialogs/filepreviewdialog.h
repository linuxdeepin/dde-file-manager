/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef FILEPREVIEWDIALOG_H
#define FILEPREVIEWDIALOG_H

#include "dfmglobal.h"
#include "durl.h"
#include "dfmfilepreview.h"
#include "dabstractfileinfo.h"
#include <dabstractdialog.h>
#include <DHorizontalLine>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DWindowCloseButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class FilePreviewDialogStatusBar;
class DFileStatisticsJob;

class UnknowFilePreview : public DFMFilePreview
{
    Q_OBJECT
public:
    explicit UnknowFilePreview(QObject *parent = nullptr);
    ~UnknowFilePreview() override;

    bool setFileUrl(const DUrl &url) override;
    DUrl fileUrl() const override;
    void setFileInfo(const DAbstractFileInfoPointer &info);

    QWidget *contentWidget() const override;

signals:
    void requestStartFolderSize();

public slots:
    void updateFolderSize(qint64 size);

private:
    DUrl m_url;
    QPointer<QWidget> m_contentWidget;
    QLabel *m_iconLabel;
    QLabel *m_nameLabel;
    QLabel *m_sizeLabel;
    QLabel *m_typeLabel;
    DFileStatisticsJob *m_sizeWorker = nullptr;
};


class FilePreviewDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit FilePreviewDialog(const DUrlList &previewUrllist, QWidget *parent = nullptr);
    ~FilePreviewDialog() override;

    void updatePreviewList(const DUrlList &previewUrllist);

    void setEntryUrlList(const DUrlList &entryUrlList);

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

    DUrlList m_fileList;
    DUrlList m_entryUrlList;

    DWindowCloseButton *m_closeButton = nullptr;
    DHorizontalLine *m_separator;
    FilePreviewDialogStatusBar *m_statusBar;

    bool m_playingVideo = false;
    bool m_firstEnterSwitchToPage = false;
    int m_currentPageIndex = -1;
    DFMFilePreview *m_preview = nullptr;

};

DFM_END_NAMESPACE

#endif // FILEPREVIEWDIALOG_H
