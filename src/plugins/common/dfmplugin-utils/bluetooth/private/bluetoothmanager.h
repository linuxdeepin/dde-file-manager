// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QDBusObjectPath>
#include <QSharedPointer>

#define BluetoothManagerInstance BluetoothManager::instance()

namespace dfmplugin_utils {

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
    bool isLongFilenameFailure(const QString &sessionPath) const;

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
}

#endif   // BLUETOOTHMANAGER_H
