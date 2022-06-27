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
#ifndef LOADINGLABEL_H
#define LOADINGLABEL_H

#include "ddplugin_wallpapersetting_global.h"

#include <dtkwidget_global.h>
#include <QFrame>

class QLabel;
DWIDGET_BEGIN_NAMESPACE
class DSpinner;
DWIDGET_END_NAMESPACE

namespace ddplugin_wallpapersetting {

class LoadingLabel : public QFrame
{
public:
    explicit LoadingLabel(QWidget *parent = nullptr);
    ~LoadingLabel();
    void start();
    void resize(const QSize &size);
    void setText(const QString &text);
private:
    void init();
    void resize(int w, int h) = delete;
private:
    QLabel *icon = nullptr;
    QLabel *contant = nullptr;
    DTK_WIDGET_NAMESPACE::DSpinner *animationSpinner = nullptr;
    double proportion = 874 / 1900.0;//根据设计计算比例
    double moveDistance = 0.0;//移动的相对位置
    QSize iconSize = QSize(30, 30);
    QSize contantSize = QSize(200, 20);
    QSize spinnerSize = QSize(24, 24);
};

}

#endif // LOADINGLABEL_H
