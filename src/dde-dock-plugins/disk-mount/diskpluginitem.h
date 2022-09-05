// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
