/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "dde-dock/constants.h"

#include <QWidget>
#include <QPixmap>

class DiskPluginItem : public QWidget
{
    Q_OBJECT
public:
    explicit DiskPluginItem(QWidget *parent = nullptr);

public slots:
    void setDockDisplayMode(const Dock::DisplayMode mode);
    void updateIcon();
    QIcon getIcon();

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    QSize sizeHint() const override;

private:
    Dock::DisplayMode displayMode;
    QPixmap icon;
};

#endif   // DISKPLUGINITEM_H
