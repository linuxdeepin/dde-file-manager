/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef WALLPAPERSERVICE_P_H
#define WALLPAPERSERVICE_P_H

#include "wallpapersetting/wallpaperservice.h"

DSB_D_BEGIN_NAMESPACE
class WallpaperServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit WallpaperServicePrivate(WallpaperService *parent);
public slots:
private:
    WallpaperService *q;

};

DSB_D_END_NAMESPACE

#endif // WALLPAPERSERVICE_P_H
