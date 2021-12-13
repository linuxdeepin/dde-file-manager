/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#ifndef BACKGROUNDDEFAULT_H
#define BACKGROUNDDEFAULT_H

#include "dfm-base/widgets/abstractbackground.h"
#include "dfm_desktop_service_global.h"

DSB_D_BEGIN_NAMESPACE

class ScreenService;

class BackgroundDefault: public dfmbase::AbstractBackground
{
    Q_OBJECT
public:
    explicit BackgroundDefault(const QString &screenName, QWidget *parent = nullptr);
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void setMode(int mode) override;
    virtual void setDisplay(const QString &path) override;
    virtual void updateDisplay() override;

private:
    QPixmap getPixmap(const QString &path, const QPixmap &defalutPixmap);

private:
    ScreenService *screenService = nullptr;

    QPixmap pixmap;
    QPixmap noScalePixmap;
};

DSB_D_END_NAMESPACE

#endif // BACKGROUNDDEFAULT_H
