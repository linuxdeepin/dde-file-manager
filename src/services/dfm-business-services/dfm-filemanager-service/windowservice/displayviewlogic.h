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
#ifndef DFMDISPLAYVIEW_H
#define DFMDISPLAYVIEW_H

#include "dfm-base/base/dfmurlroute.h"
#include "dfm_filemanager_service_global.h"

#include <QUrl>
#include <QWidget>

DSB_FM_BEGIN_NAMESPACE

/* @class DFMDisplayViewLogic
 * @brief 展示的view界面共有逻辑类
 * 框架执行流程为：
 *      1.checkViewUrl
 *      2.setRootUrl
 *      3.showBeginLogic
 *      4.showEndLogic
 */
class DFMDisplayViewLogic
{
    Q_DISABLE_COPY(DFMDisplayViewLogic)

public:

    explicit DFMDisplayViewLogic();

    virtual ~DFMDisplayViewLogic();

    //检查当前传入的url是否被view支持
    virtual bool checkViewUrl(const QUrl& url, QString *errorString = nullptr);

    //设置url
    virtual void setRootUrl(const QUrl &url) = 0;

    //获取url
    virtual QUrl rootUrl() = 0;

    //run in main loop,cause show dialog and so on.
    //展示view之前逻辑
    virtual void showBeginLogic();

    //用于view切换关闭的逻辑
    //run in main loop, hide self widget
    virtual void showEndLogic();
};

typedef QSharedPointer<DFMDisplayViewLogic> DFMDisplayViewLogicPointer;

DSB_FM_END_NAMESPACE

#endif // DFMDISPLAYVIEW_H
