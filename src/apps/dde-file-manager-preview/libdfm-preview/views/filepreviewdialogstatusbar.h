// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEWDIALOGSTATUSBAR_H
#define FILEPREVIEWDIALOGSTATUSBAR_H

#include "dfmplugin_filepreview_global.h"

#include <QFrame>
#include <QLabel>
#include <QPushButton>

QT_BEGIN_NAMESPACE
class QEvent;
QT_END_NAMESPACE

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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updateNavButtonIcons();

    QLabel *previewTitle { nullptr };

    QPushButton *preBtn { nullptr };
    QPushButton *nextBtn { nullptr };
    QPushButton *openBtn { nullptr };

    bool preBtnHovered { false };
    bool nextBtnHovered { false };
};
}
#endif   // FILEPREVIEWDIALOGSTATUSBAR_H
