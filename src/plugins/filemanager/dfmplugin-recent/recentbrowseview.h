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
#ifndef RECENTBROWSEVIEW_H
#define RECENTBROWSEVIEW_H

#include "recentutil.h"
#include "window/browseview.h" // TODO(zhangs): hide
#include "recentbrowseviewmodel.h"

DSB_FM_USE_NAMESPACE

class RecentBrowseView : public BrowseView
{
public:
    explicit RecentBrowseView(QWidget *parent = nullptr);
    virtual void setRootUrl(const QUrl &url) override;
    virtual QUrl rootUrl() override;
};

#endif // RECENTBROWSEVIEW_H
