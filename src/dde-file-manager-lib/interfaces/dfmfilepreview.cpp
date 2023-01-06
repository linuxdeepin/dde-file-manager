// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmfilepreview.h"
#include "dfmglobal.h"

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

void DFMFilePreview::DoneCurrent()
{

}

void DFMFilePreview::play()
{

}

void DFMFilePreview::pause()
{

}

void DFMFilePreview::stop()
{

}

void DFMFilePreview::copyFile() const
{
    DFMGlobal::setUrlsToClipboard({fileUrl()}, DFMGlobal::CopyAction);
}

DFM_END_NAMESPACE
