/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
