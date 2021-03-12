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

#ifndef DISKCONTROLITEM_H
#define DISKCONTROLITEM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <dfmglobal.h>
#include "dattacheddeviceinterface.h"
#include "dtkwidget_global.h"

DFM_BEGIN_NAMESPACE
class DFMSettings;
DFM_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DiskControlItem : public QFrame
{
    Q_OBJECT

public:
    explicit DiskControlItem(DAttachedDeviceInterface *attachedDevicePtr, QWidget *parent = nullptr);
    ~DiskControlItem() override;

    static QString formatDiskSize(const quint64 num);
    void detachDevice();
    QString driveName();
    QString tagName() const;
    void setTagName(const QString &tagName);
    QUrl mountPointUrl();

signals:
    void umountClicked(DiskControlItem *item);

private slots:
    static QString sizeString(const QString &str);

private:
    void mouseReleaseEvent(QMouseEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void refreshIcon();

private:
    QIcon m_unknowIcon;

    QPushButton *m_diskIcon;
    QLabel *m_diskName;
    QLabel *m_diskCapacity;
    QProgressBar *m_capacityValueBar;
    DIconButton *m_unmountButton;
    QScopedPointer<DAttachedDeviceInterface> attachedDevice;

    QString m_tagName;
};

DFM_NAMESPACE::DFMSettings* getGsGlobal();

#endif // DISKCONTROLITEM_H
