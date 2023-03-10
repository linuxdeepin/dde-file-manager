// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKCONTROLITEM_H
#define DISKCONTROLITEM_H

#include "device/dattacheddevice.h"

#include <DIconButton>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

class DiskControlWidget;
class DiskControlItem : public QFrame
{
    Q_OBJECT
public:
    explicit DiskControlItem(QSharedPointer<DAttachedDevice> attachedDevicePtr, QWidget *parent = nullptr);

private slots:
    void detachDevice();

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    void initializeUi();
    void initConnection();
    void refreshIcon();

private:
    QIcon unknowIcon;
    QPushButton *diskIcon { nullptr };
    QLabel *diskName { nullptr };
    QLabel *diskCapacity { nullptr };
    QProgressBar *capacityValueBar { nullptr };
    QSharedPointer<DAttachedDevice> attachedDev;
    DTK_WIDGET_NAMESPACE::DIconButton *ejectButton { nullptr };
    DiskControlWidget *par;
};

#endif   // DISKCONTROLITEM_H
