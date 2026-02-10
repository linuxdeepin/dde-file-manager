// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICELIST_H
#define DEVICELIST_H

#include "device/typedefines.h"

#include <QObject>
#include <QMap>
#include <QScrollArea>
#include <QShowEvent>

class DockItemDataManager;
class QVBoxLayout;
class DeviceList : public QScrollArea
{
    Q_OBJECT
public:
    explicit DeviceList(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;

private Q_SLOTS:
    void addDevice(const DockItemData &item);
    void removeDevice(const QString &id);
    void ejectDevice(const QString &id);

private:
    void initUI();
    void initConnect();
    void updateHeight();
    QWidget *createHeader();

private:
    QVBoxLayout *deviceLay { nullptr };
    DockItemDataManager *manager { nullptr };
    QMap<QString, QWidget *> deviceItems;
    QMap<QString, QString> sortKeys;
    QWidget *headerWidget { nullptr };
};

#endif   // DEVICELIST_H
