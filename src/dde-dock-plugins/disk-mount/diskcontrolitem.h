/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DISKCONTROLITEM_H
#define DISKCONTROLITEM_H

#include "dattacheddeviceinterface.h"


#include <DIconButton>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

class DiskControlItem : public QFrame
{
    Q_OBJECT
public:
    explicit DiskControlItem(QSharedPointer<DAttachedDeviceInterface> attachedDevicePtr, QWidget *parent = nullptr);

signals:
    void umountClicked(DiskControlItem *item);

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    void initializeUi();
    void initConnection();
    void refreshIcon();
    QString sizeString(const QString &str);

private:
    QIcon unknowIcon;
    QPushButton *diskIcon {nullptr};
    QLabel *diskName {nullptr};
    QLabel *diskCapacity {nullptr};
    QProgressBar *capacityValueBar {nullptr};
    QSharedPointer<DAttachedDeviceInterface> dev;
    DTK_WIDGET_NAMESPACE::DIconButton *unmountButton {nullptr};
};

#endif // DISKCONTROLITEM_H
