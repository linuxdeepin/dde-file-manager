/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FMSTATE_H
#define FMSTATE_H

#include <QObject>
#include <QSize>
#include "debugobejct.h"

class FMState : public DebugObejct
{
    Q_OBJECT

    Q_PROPERTY(int m_viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(int m_x READ x WRITE setX)
    Q_PROPERTY(int m_y READ y WRITE setY)
    Q_PROPERTY(int m_width READ width WRITE setWidth)
    Q_PROPERTY(int m_height READ height WRITE setHeight)
    Q_PROPERTY(int m_windowState READ windowState WRITE setWindowState)
    Q_PROPERTY(QString m_theme READ theme WRITE setTheme)
public:
    explicit FMState(QObject *parent = 0);
    ~FMState();

    int viewMode() const;
    void setViewMode(int viewMode);

    QSize size() const;
    void setSize(const QSize &size);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

    int windowState() const;
    void setWindowState(int windowState);

    QString theme() const;
    void setTheme(const QString &theme);

signals:

public slots:

private:
    int m_viewMode = 1;
    int m_x = 0;
    int m_y = 0;
    int m_width = 950;
    int m_height = 600;
    int m_windowState = 0;
    QString m_theme = "light";
};


#endif // FMSTATE_H
