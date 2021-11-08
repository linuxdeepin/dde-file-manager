/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "displayviewlogic.h"

DSB_FM_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

/**
 * @class DFMDisplayViewLogic
 * @brief 展示的view界面共有逻辑类
 * 框架执行流程为：
 *      1.checkViewUrl
 *      2.setRootUrl
 *      3.showBeginLogic
 *      4.showEndLogic
 */
DisplayViewLogic::DisplayViewLogic()
{

}

DisplayViewLogic::~DisplayViewLogic()
{

}

//检查当前传入的url是否被view支持
bool DisplayViewLogic::checkViewUrl(const QUrl &url, QString *errorString)
{
    if(url.scheme().isEmpty() || !UrlRoute::hasScheme(url.scheme()))
    {
        if(errorString)
            *errorString = QObject::tr("Url is not supported, scheme is %0").arg(url.scheme());
        return false;
    }
    return true;
}

//run in main loop,cause show dialog and so on.
//展示view之前逻辑
void DisplayViewLogic::showBeginLogic()
{
    auto selfWidget = dynamic_cast<QWidget*>(this);
    if (selfWidget) {
        selfWidget->show();
    }
}

//用于view切换关闭的逻辑
//run in main loop, hide self widget
void DisplayViewLogic::showEndLogic()
{
    auto selfWidget = dynamic_cast<QWidget*>(this);
    if (selfWidget) {
        selfWidget->hide();
    }
}

DSB_FM_END_NAMESPACE
