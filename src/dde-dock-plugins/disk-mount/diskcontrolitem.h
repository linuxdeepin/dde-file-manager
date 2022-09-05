// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
