// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    setWindowTitle("Desktop");
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowIcon(QIcon::fromTheme("deepin-toggle-desktop"));
}
