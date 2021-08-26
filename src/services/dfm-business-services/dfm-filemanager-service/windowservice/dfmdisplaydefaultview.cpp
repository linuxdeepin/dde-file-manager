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
#include "dfmdisplaydefaultview.h"

DFMDisplayDefaultView::DFMDisplayDefaultView(QWidget *parent)
    : QLabel (parent)
{
    setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
}

void DFMDisplayDefaultView::setRootUrl(const QUrl &url)
{
    m_url = url;
}

QUrl DFMDisplayDefaultView::rootUrl()
{
    return m_url;
}

bool DFMDisplayDefaultView::checkViewUrl(const QUrl &url, QString *errorString)
{
    Q_UNUSED(errorString)

    if (!url.isValid() || url.isEmpty())
    {
        this->setText(QObject::tr("Url is not supported"));
        return false; //return true 會執行showBeginLogi函数
    }
    return true;
}

void DFMDisplayDefaultView::showBeginLogic()
{
    //直接展示
    DFMDisplayViewLogic::showBeginLogic();
}

void DFMDisplayDefaultView::showEndLogic()
{
    //直接隐藏
    DFMDisplayViewLogic::showEndLogic();
}
