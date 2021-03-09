/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#ifndef SCREENOBJECT_H
#define SCREENOBJECT_H

#include "abstractscreen.h"
#include <QScreen>

class ScreenObject : public AbstractScreen
{
public:
    ScreenObject(QScreen *sc, QObject *parent = nullptr);
    ~ScreenObject() override;
    QString name() const override;
    QRect geometry() const override;
    QRect availableGeometry() const override;
    QRect handleGeometry() const override;
public:
    QScreen *screen() const;
private:
    void init();
private:
    QScreen *m_screen = nullptr;
};

typedef QSharedPointer<ScreenObject> ScreenObjectPointer;
#endif // SCREENOBJECT_H
