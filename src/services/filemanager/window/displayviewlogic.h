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
#ifndef DFMDISPLAYVIEW_H
#define DFMDISPLAYVIEW_H

#include "dfm-base/base/urlroute.h"
#include "dfm_filemanager_service_global.h"

#include <QUrl>
#include <QWidget>

DSB_FM_BEGIN_NAMESPACE

class DisplayViewLogic
{
    Q_DISABLE_COPY(DisplayViewLogic)
public:
    explicit DisplayViewLogic();
    virtual ~DisplayViewLogic();
    virtual bool checkViewUrl(const QUrl& url, QString *errorString = nullptr);
    virtual void setRootUrl(const QUrl &url) = 0;    //设置url
    virtual QUrl rootUrl() = 0;    //获取url
    virtual void showBeginLogic();
    virtual void showEndLogic();
};

typedef QSharedPointer<DisplayViewLogic> DFMDisplayViewLogicPointer;

DSB_FM_END_NAMESPACE

#endif // DFMDISPLAYVIEW_H
