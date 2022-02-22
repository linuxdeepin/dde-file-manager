/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             lvwujun<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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

#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include "dfm_common_service_global.h"

#include <QObject>
#include <QDBusObjectPath>
#include <QSharedPointer>

#define BluetoothManagerInstance BluetoothManager::instance()

DSC_BEGIN_NAMESPACE
class BluetoothAdapter;
class BluetoothManagerPrivate;

/**
 * @brief This is singleton class.
 * you should `#include "app/define.h"` and `"bluetooth/bluetoothmanager.h"`,
 * then use the macro `BluetoothManagerInstance` call method
 */
class BluetoothManager : public QObject
{
    Q_OBJECT

public:
    static BluetoothManager *instance();
    QMap<QString, const BluetoothAdapter *> getAdapters() const;
    bool hasAdapter();
    bool bluetoothSendEnable();

public Q_SLOTS:
    void refresh();
    void showBluetoothSettings();
    void sendFiles(const QString &id, const QStringList &filePath, const QString &senderToken);
    bool cancelTransfer(const QString &sessionPath);
    bool canSendBluetoothRequest();

Q_SIGNALS:
    void transferProgressUpdated(const QString &sessionPath, qulonglong total, qulonglong transferred, int currFileIndex);
    void transferCancledByRemote(const QString &sessionPath);
    void fileTransferFinished(const QString &sessionPath, const QString &filePath);
    void transferFailed(const QString &sessionPath, const QString &filePath, const QString &errMsg);
    void transferEstablishFinish(const QString &sessionPath, const QString &errMsg, const QString &senderToken);
    void adapterAdded(const BluetoothAdapter *adapter) const;
    void adapterRemoved(const BluetoothAdapter *adapter) const;

private:
    explicit BluetoothManager(QObject *parent = nullptr);
    BluetoothManager(const BluetoothManager &) = delete;
    BluetoothManager &operator=(const BluetoothManager &) = delete;

    QSharedPointer<BluetoothManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(BluetoothManager)
};
DSC_END_NAMESPACE

#endif   // BLUETOOTHMANAGER_H
