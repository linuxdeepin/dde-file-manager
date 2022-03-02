/*
 * Copyright (C) 2022s Uniontech Software Technology Co., Ltd.
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
#ifndef WIDGETUTIL_H
#define WIDGETUTIL_H

#include <QWidget>
#include <QApplication>

namespace ddplugin_desktop_util {

bool enableScreensaver();
void setDesktopWindow(QWidget *w);
void setPrviewWindow(QWidget *w);

}

#endif // WIDGETUTIL_H
