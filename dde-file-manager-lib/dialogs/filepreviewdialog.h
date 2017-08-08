/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef FILEPREVIEWDIALOG_H
#define FILEPREVIEWDIALOG_H

#include "dfmglobal.h"
#include "durl.h"

#include <dabstractdialog.h>
#include <dseparatorhorizontal.h>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class FilePreviewDialogStatusBar;
class DFMFilePreview;
class FilePreviewDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit FilePreviewDialog(const DUrlList &list, QWidget *parent = 0);
    ~FilePreviewDialog();
    bool isCurrentMusicPreview();

private:
    void childEvent(QChildEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

    void initUI();
    void switchToPage(int index);
    void previousPage();
    void nextPage();

    void updateTitle();

    DUrlList m_fileList;

    QPushButton *m_closeButton = 0;
    DSeparatorHorizontal *m_separator;
    FilePreviewDialogStatusBar *m_statusBar;

    int m_currentPageIndex = -1;
    DFMFilePreview *m_preview = 0;
};

DFM_END_NAMESPACE

#endif // FILEPREVIEWDIALOG_H
