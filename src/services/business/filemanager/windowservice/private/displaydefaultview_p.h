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
#ifndef DISPLAYDEFAULTVIEW_P_H
#define DISPLAYDEFAULTVIEW_P_H

#include "dfm_filemanager_service_global.h"

#include <QUrl>
#include <QObject>

DSB_FM_BEGIN_NAMESPACE

class DisplayDefaultView;
class DisplayDefaultViewPrivate : public QObject
{
    Q_OBJECT
    friend class DisplayDefaultView;
    DisplayDefaultView *const q;
    QUrl url;

    explicit DisplayDefaultViewPrivate(DisplayDefaultView *qq);
};

DSB_FM_END_NAMESPACE

#endif // DISPLAYDEFAULTVIEW_P_H
