// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basewindow.h"

#include <QIcon>

DDPCORE_USE_NAMESPACE

BaseWindow::BaseWindow(QWidget *parent)
    : QWidget(parent)
{
}

void BaseWindow::init()
{
    setWindowTitle(QObject::tr("Desktop"));
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowIcon(QIcon::fromTheme("deepin-toggle-desktop"));
}

void BaseWindow::recreateNativeWindow(const QRect &geometry)
{
    hide();
    setGeometry(geometry);
    destroy(true, false);
    create();
    setGeometry(geometry);
}
