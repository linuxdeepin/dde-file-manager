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
#ifndef DFMDISPLAYDEFAULTVIEW_H
#define DFMDISPLAYDEFAULTVIEW_H

#include "displayviewlogic.h"
#include "dfm_filemanager_service_global.h"

#include <QLabel>

DSB_FM_BEGIN_NAMESPACE

/* @class DFMDisplayViewDefault
 * @brief 默认view的展示界面，主要提供url入口检查与界面提示。
 */
class DFMDisplayDefaultView : public QLabel, public DFMDisplayViewLogic
{
    Q_OBJECT
    Q_DISABLE_COPY(DFMDisplayDefaultView)

    QUrl m_url;

public:
    explicit DFMDisplayDefaultView(QWidget* parent = nullptr);

    //设置url
    virtual void setRootUrl(const QUrl &url) override;

    //获取url
    virtual QUrl rootUrl() override;

    //重载实现现有url检查,实现界面的View弹出提示
    virtual bool checkViewUrl(const QUrl& url, QString *errorString = nullptr) override;

    //run in main loop,cause show dialog and so on.
    //展示逻辑
    virtual void showBeginLogic() override;

    //用于关闭的逻辑
    //run in main loop, hide self widget
    virtual void showEndLogic() override;
};

DSB_FM_END_NAMESPACE

#endif // DFMDISPLAYDEFAULTVIEW_H
