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

#ifndef DISKCONTROLWIDGET_H
#define DISKCONTROLWIDGET_H

#include <QScrollArea>
#include <QVBoxLayout>

class DFMSetting;
class GvfsMountManager;
class QDiskInfo;
class QDrive;


class DiskControlWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit DiskControlWidget(QWidget *parent = 0);
    void initConnect();

    void startMonitor();
    void unmountAll();

signals:
    void diskCountChanged(const int count) const;

private slots:
    void onDiskListChanged();
    void onDrive_connected(const QDrive& drive);
    void onDrive_disconnected(const QDrive& drive);
    void onMount_added(const QDiskInfo &diskInfo);
    void onMount_removed(const QDiskInfo &diskInfo);
    void onVolume_added(const QDiskInfo &diskInfo);
    void onVolume_removed(const QDiskInfo &diskInfo);
    void onVolume_changed(const QDiskInfo &diskInfo);
    void unmountDisk(const QString &diskId) const;

private:
    QVBoxLayout *m_centralLayout;
    QWidget *m_centralWidget;
    GvfsMountManager *m_gvfsMountManager;
};

#endif // DISKCONTROLWIDGET_H
