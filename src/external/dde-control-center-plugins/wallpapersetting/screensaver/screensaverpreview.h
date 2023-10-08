// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSAVERPREVIEW_H
#define SCREENSAVERPREVIEW_H

#include "common/previewwidget.h"

class QPushButton;
namespace dfm_wallpapersetting {

class ScreensaverPreview : public PreviewWidget
{
    Q_OBJECT
public:
    explicit ScreensaverPreview(QWidget *parent = nullptr);
signals:
    void clicked();
protected:
    bool event(QEvent *event) override;
protected:
    QPushButton *detail = nullptr;
};

}

#endif // SCREENSAVERPREVIEW_H
