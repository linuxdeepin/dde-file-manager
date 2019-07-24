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

#ifndef DISKCONTROLITEM_H
#define DISKCONTROLITEM_H

#include <dimagebutton.h>

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <dfmglobal.h>
#include "dattacheddeviceinterface.h"

DFM_BEGIN_NAMESPACE
class DFMSettings;
DFM_END_NAMESPACE

class DiskControlItem : public QFrame
{
    Q_OBJECT

public:
    explicit DiskControlItem(DAttachedDeviceInterface *attachedDevicePtr, QWidget *parent = nullptr);
    ~DiskControlItem() override;

    static QString formatDiskSize(const quint64 num);

private slots:
    static QString sizeString(const QString &str);

private:
    void mouseReleaseEvent(QMouseEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    QIcon m_unknowIcon;

    QPushButton *m_diskIcon;
    QLabel *m_diskName;
    QLabel *m_diskCapacity;
    QProgressBar *m_capacityValueBar;
    Dtk::Widget::DImageButton *m_unmountButton;
    QScopedPointer<DAttachedDeviceInterface> attachedDevice;
};

DFM_NAMESPACE::DFMSettings* getGsGlobal();

#endif // DISKCONTROLITEM_H
