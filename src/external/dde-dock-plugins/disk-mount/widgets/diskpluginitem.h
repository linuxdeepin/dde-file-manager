// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

private:
    Dock::DisplayMode displayMode;
    QPixmap icon;
};

#endif   // DISKPLUGINITEM_H
