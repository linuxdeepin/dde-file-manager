// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screensaverpreview.h"
#include "common/commondefine.h"

#include <QEvent>
#include <QPushButton>
#include <QDebug>

using namespace dfm_wallpapersetting;

ScreensaverPreview::ScreensaverPreview(QWidget *parent) : PreviewWidget(parent)
{
    detail = new QPushButton(this);
    detail->setText(tr("Full-Screen"));
    detail->setFixedSize(141, 36);
    detail->setFocusPolicy(Qt::NoFocus);
    detail->move(PREVIEW_ICON_MARGIN + 53, PREVIEW_ICON_MARGIN + 93);
    detail->hide();
    connect(detail, &QPushButton::clicked, this , &ScreensaverPreview::clicked);
}

bool ScreensaverPreview::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        detail->show();
        break;
    case QEvent::Leave:
        detail->hide();
        break;
    default:
        break;
    }

    return PreviewWidget::event(event);
}
