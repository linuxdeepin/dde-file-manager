/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "private/displaydefaultview_p.h"
#include "displaydefaultview.h"

DSB_FM_BEGIN_NAMESPACE

DisplayDefaultViewPrivate::DisplayDefaultViewPrivate(DisplayDefaultView *qq)
    : QObject(qq)
    , q_ptr(qq)
{

}

/**
 * @class DFMDisplayViewDefault
 * @brief 默认view的展示界面，主要提供url入口检查与界面提示。
 */
DisplayDefaultView::DisplayDefaultView(QWidget *parent)
    : QLabel (parent)
    , d(new DisplayDefaultViewPrivate(this))
{
    setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
}

// 设置url
void DisplayDefaultView::setRootUrl(const QUrl &url)
{
    d->url = url;
}

// 获取url
QUrl DisplayDefaultView::rootUrl()
{
    return d->url;
}

// 重载实现现有url检查,实现界面的View弹出提示
bool DisplayDefaultView::checkViewUrl(const QUrl &url, QString *errorString)
{
    Q_UNUSED(errorString)

    if (!url.isValid() || url.isEmpty())
    {
        this->setText(QObject::tr("Url is not supported"));
        return false; //return true 會執行showBeginLogi函数
    }
    return true;
}

//展示逻辑
//run in main loop,cause show dialog and so on.
void DisplayDefaultView::showBeginLogic()
{
    //直接展示
    DisplayViewLogic::showBeginLogic();
}

//用于关闭的逻辑
//run in main loop, hide self widget
void DisplayDefaultView::showEndLogic()
{
    //直接隐藏
    DisplayViewLogic::showEndLogic();
}

DSB_FM_END_NAMESPACE

