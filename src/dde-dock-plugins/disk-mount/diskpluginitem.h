/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef DISKPLUGINITEM_H
#define DISKPLUGINITEM_H

#include "constants.h"

#include <QWidget>
#include <QPixmap>

#define DISK_MOUNT_KEY "mount-item-key"

class DiskPluginItem : public QWidget
{
    Q_OBJECT

public:
    explicit DiskPluginItem(QWidget *parent = nullptr);

public slots:
    void setDockDisplayMode(const Dock::DisplayMode mode);
    void updateIcon();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    QSize sizeHint() const;

private:
    Dock::DisplayMode m_displayMode;

    QPixmap m_icon;
};

#endif // DISKPLUGINITEM_H
