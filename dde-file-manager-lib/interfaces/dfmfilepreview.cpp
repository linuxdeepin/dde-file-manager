/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "dfmfilepreview.h"

DFM_BEGIN_NAMESPACE

DFMFilePreview::DFMFilePreview(QObject *parent)
    : QObject(parent)
{

}

void DFMFilePreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(window)
    Q_UNUSED(statusBar)
}

QWidget *DFMFilePreview::statusBarWidget() const
{
    return 0;
}

Qt::Alignment DFMFilePreview::statusBarWidgetAlignment() const
{
    return Qt::AlignCenter;
}

QString DFMFilePreview::title() const
{
    return QString();
}

bool DFMFilePreview::showStatusBarSeparator() const
{
    return false;
}

DFM_END_NAMESPACE
