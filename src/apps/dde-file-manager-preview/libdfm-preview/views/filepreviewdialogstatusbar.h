// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEWDIALOGSTATUSBAR_H
#define FILEPREVIEWDIALOGSTATUSBAR_H

#include "dfmplugin_filepreview_global.h"

#include <QFrame>
#include <QLabel>
#include <QPushButton>

namespace dfmplugin_filepreview {
class FilePreviewDialogStatusBar : public QFrame
{
    Q_OBJECT
public:
    explicit FilePreviewDialogStatusBar(QWidget *parent = nullptr);

    QLabel *title() const;
    QPushButton *preButton() const;
    QPushButton *nextButton() const;
    QPushButton *openButton() const;

private:
    QLabel *previewTitle { nullptr };

    QPushButton *preBtn { nullptr };
    QPushButton *nextBtn { nullptr };
    QPushButton *openBtn { nullptr };
};
}
#endif   // FILEPREVIEWDIALOGSTATUSBAR_H
