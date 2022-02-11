/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "bluetoothmanager_p.h"
#include "bluetooth/bluetoothmodel.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#define BluetoothService "com.deepin.daemon.Bluetooth"
#define BluetoothPath "/com/deepin/daemon/Bluetooth"
#define BlutoothInterface "com.deepin.daemon.Bluetooth"

#define BluetoothPage "bluetooth"
#define ControlcenterService "com.deepin.dde.ControlCenter"
#define ControlcenterPath "/com/deepin/dde/ControlCenter"

BluetoothManagerPrivate::BluetoothManagerPrivate(BluetoothManager *qq)
    : q_ptr(qq),
      m_model(new BluetoothModel(qq))
{
    Q_Q(BluetoothManager);
    // initialize dbus interface
    m_bluetoothInter = new DBusBluetooth(BluetoothService, BluetoothPath,
                                         QDBusConnection::sessionBus(), q);
    m_controlcenterInter = new DBusControlcenter(ControlcenterService, ControlcenterPath,
                                                 QDBusConnection::sessionBus(), q);

    initConnects();
}

void BluetoothManagerPrivate::resolve(const QDBusReply<QString> &req)
{
    Q_Q(BluetoothManager);
    const QString replyStr = req.value();
    qInfo() << replyStr;
    static int maxRetry = 3;
    // GetAdapter method, if there has no adapter, the method returns an emtpy json [], but if the method is not ready,
    // returns a null string. if null, retry
    if (replyStr.isEmpty() && maxRetry > 0) {
        qInfo() << "retry to get bluetooth adapters..." << maxRetry;
        QTimer::singleShot(500, q, [q]{
            q->refresh();
        });
        maxRetry--;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(replyStr.toUtf8());
    QJsonArray arr = doc.array();
    for (QJsonValue val : arr) {
        BluetoothAdapter *adapter = new BluetoothAdapter(m_model);
        inflateAdapter(adapter, val.toObject());
        m_model->addAdapter(adapter);
    }
}

void BluetoothManagerPrivate::initConnects()
{
    Q_Q(BluetoothManager);

    QObject::connect(m_bluetoothInter, &DBusBluetooth::serviceValidChanged, q, [q](bool valid){
        if (valid) {
            qInfo() << "bluetooth service is valid now...";
            QTimer::singleShot(1000, q, [q]{ q->refresh(); });
        }
    });

    // adapter added
    QObject::connect(m_bluetoothInter, &DBusBluetooth::AdapterAdded, q, [=](const QString & json) {
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        QJsonObject obj = doc.object();

        BluetoothAdapter *adapter = new BluetoothAdapter(m_model);
        inflateAdapter(adapter, obj);
        m_model->addAdapter(adapter);
    });

    // adapter removed
    QObject::connect(m_bluetoothInter, &DBusBluetooth::AdapterRemoved, q, [=](const QString & json) {
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        QJsonObject obj = doc.object();
        const QString id = obj["Path"].toString();

        const BluetoothAdapter *result = m_model->removeAdapater(id);
        BluetoothAdapter *adapter = const_cast<BluetoothAdapter *>(result);
        if (adapter) {
            adapter->deleteLater();
        }
    });

    // adapter changed
    QObject::connect(m_bluetoothInter, &DBusBluetooth::AdapterPropertiesChanged, q, [=](const QString & json) {
        const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        const QJsonObject obj = doc.object();
        const QString id = obj["Path"].toString();

        BluetoothAdapter *adapter = const_cast<BluetoothAdapter *>(m_model->adapterById(id));
        if (adapter) {
            inflateAdapter(adapter, obj);
        }
    });

    // device added
    QObject::connect(m_bluetoothInter, &DBusBluetooth::DeviceAdded, q, [=](const QString & json) {
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        QJsonObject obj = doc.object();
        const QString adapterId = obj["AdapterPath"].toString();
        const QString id = obj["Path"].toString();

        const BluetoothAdapter *result = m_model->adapterById(adapterId);
        BluetoothAdapter *adapter = const_cast<BluetoothAdapter *>(result);
        if (adapter) {
            const BluetoothDevice *result1 = adapter->deviceById(id);
            BluetoothDevice *device = const_cast<BluetoothDevice *>(result1);
            if (!device) {
                device = new BluetoothDevice(adapter);
            }
            inflateDevice(device, obj);
            adapter->addDevice(device);
        }
    });

    // deivce removeed
    QObject::connect(m_bluetoothInter, &DBusBluetooth::DeviceRemoved, q, [=](const QString & json) {
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        QJsonObject obj = doc.object();
        const QString adapterId = obj["AdapterPath"].toString();
        const QString id = obj["Path"].toString();

        const BluetoothAdapter *result = m_model->adapterById(adapterId);
        BluetoothAdapter *adapter = const_cast<BluetoothAdapter *>(result);
        if (adapter) {
            adapter->removeDevice(id);
        }
    });

    // device changed
    QObject::connect(m_bluetoothInter, &DBusBluetooth::DevicePropertiesChanged, q, [this](const QString & json) {
        const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        const QJsonObject obj = doc.object();
        const QString id = obj["Path"].toString();
        for (const BluetoothAdapter *adapter : m_model->adapters()) {
            BluetoothDevice *device = const_cast<BluetoothDevice *>(adapter->deviceById(id));
            if (device) {
                inflateDevice(device, obj);
            }
        }
    });

    QObject::connect(m_bluetoothInter, &DBusBluetooth::TransferCreated, q, [=](const QString & file, const QDBusObjectPath & transferPath, const QDBusObjectPath & sessionPath) {
        qDebug() << file << transferPath.path() << sessionPath.path();
    });

    QObject::connect(m_bluetoothInter, &DBusBluetooth::TransferRemoved, q, [=](const QString & file, const QDBusObjectPath & transferPath, const QDBusObjectPath & sessionPath, bool done) {
        Q_UNUSED(transferPath)
        Q_Q(BluetoothManager);
        if (!done) {
            Q_EMIT q->transferCancledByRemote(sessionPath.path());
        } else {
            Q_EMIT q->fileTransferFinished(sessionPath.path(), file);
        }
    });

    QObject::connect(m_bluetoothInter, &DBusBluetooth::ObexSessionCreated, q, [=](const QDBusObjectPath & sessionPath) {
        qDebug() << sessionPath.path();
    });

    QObject::connect(m_bluetoothInter, &DBusBluetooth::ObexSessionRemoved, q, [=](const QDBusObjectPath & sessionPath) {
        qDebug() << sessionPath.path();
    });

    QObject::connect(m_bluetoothInter, &DBusBluetooth::ObexSessionProgress, q, [=](const QDBusObjectPath & sessionPath, qulonglong totalSize, qulonglong transferred, int currentIdx) {
        Q_Q(BluetoothManager);
        Q_EMIT q->transferProgressUpdated(sessionPath.path(), totalSize, transferred, currentIdx);
    });

    QObject::connect(m_bluetoothInter, &DBusBluetooth::TransferFailed, q, [=](const QString & file, const QDBusObjectPath & sessionPath, const QString & errInfo) {
        Q_Q(BluetoothManager);
        Q_EMIT q->transferFailed(sessionPath.path(), file, errInfo);
    });
}

void BluetoothManagerPrivate::inflateAdapter(BluetoothAdapter *adapter, const QJsonObject &adapterObj)
{
    Q_Q(BluetoothManager);

    const QString path = adapterObj["Path"].toString();
    const QString alias = adapterObj["Alias"].toString();
    const bool powered = adapterObj["Powered"].toBool();
    qDebug() << "resolve adapter path:" << path;

    adapter->setId(path);
    adapter->setName(alias);
    adapter->setPowered(powered);

    QPointer<BluetoothAdapter> adapterPointer(adapter);

    // 异步获取适配器的所有设备
    QDBusObjectPath dPath(path);
    QDBusPendingCall call = m_bluetoothInter->GetDevices(dPath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, q, [this, watcher, adapterPointer, call] {
        if (!adapterPointer) {
            qDebug() << "adapterPointer released!";
            watcher->deleteLater();
            return;
        }
        BluetoothAdapter *adapter = adapterPointer.data();
        if (!call.isError()) {
            QStringList tmpList;

            QDBusReply<QString> reply = call.reply();
            const QString replyStr = reply.value();
            QJsonDocument doc = QJsonDocument::fromJson(replyStr.toUtf8());
            QJsonArray arr = doc.array();

            for (QJsonValue val : arr) {
                const QString id = val.toObject()["Path"].toString();
                const BluetoothDevice *result = adapter->deviceById(id);
                BluetoothDevice *device = const_cast<BluetoothDevice *>(result);
                if (device == nullptr) {
                    device = new BluetoothDevice(adapter);
                }
                // 存储设备数据
                inflateDevice(device, val.toObject());
                adapter->addDevice(device);

                tmpList << id;
            }

            // 适配器设备去重
            for (const BluetoothDevice *device : adapter->devices()) {
                if (!tmpList.contains(device->id())) {
                    adapter->removeDevice(device->id());

                    BluetoothDevice *target = const_cast<BluetoothDevice *>(device);
                    if (target) {
                        target->deleteLater();
                    }
                }
            }
        } else {
            qWarning() << call.error().message();
        }

        watcher->deleteLater();
    });
}

void BluetoothManagerPrivate::inflateDevice(BluetoothDevice *device, const QJsonObject &deviceObj)
{
    const QString &id = deviceObj["Path"].toString();
    const QString &name = deviceObj["Name"].toString();
    const QString &alias = deviceObj["Alias"].toString();
    const QString &icon = deviceObj["Icon"].toString();
    const bool paired = deviceObj["Paired"].toBool();
    const bool trusted = deviceObj["Trusted"].toBool();
    const BluetoothDevice::State state = BluetoothDevice::State(deviceObj["State"].toInt());

    device->setId(id);
    device->setName(name);
    device->setAlias(alias);
    device->setIcon(icon);
    device->setPaired(paired);
    device->setTrusted(trusted);
    device->setState(state);
}

/**
 * @brief This is BluetoothManager implement
 */
BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent),
      d_ptr(new BluetoothManagerPrivate(this))
{
    refresh();
}


BluetoothManager *BluetoothManager::instance()
{
    static BluetoothManager bluetooth;
    return &bluetooth;
}


void BluetoothManager::refresh()
{
    Q_D(BluetoothManager);

    if (!d->m_bluetoothInter->isValid()) {
        qCritical() << "bluetooth interface is not valid!!!";
        return;
    }

    // 获取蓝牙设备
    QDBusPendingCall call = d->m_bluetoothInter->GetAdapters();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, [ = ] {
        if (!call.isError())
        {
            QDBusReply<QString> reply = call.reply();
            d->resolve(reply);
        } else
        {
            qWarning() << call.error().message();
        }
        watcher->deleteLater();
    });
}

BluetoothModel *BluetoothManager::model()
{
    Q_D(BluetoothManager);

    return d->m_model;
}

bool BluetoothManager::hasAdapter()
{
    return model()->adapters().count() > 0;
}

bool BluetoothManager::bluetoothSendEnable()
{
    QDBusInterface btIface(BluetoothService, BluetoothPath, BlutoothInterface, QDBusConnection::sessionBus());
    if (!btIface.isValid()) {
        qWarning() << "bluetooth interface is not valid";
        return false;
    }
    auto canSendFile = btIface.property("CanSendFile");
    if (!canSendFile.isValid()) {
        qWarning() << "bluetooth interface has no 'CanSendFile' property";
        return true; // if there is no this property, then we do not disable bluetooth transfer.
    }
    return canSendFile.toBool();
}

void BluetoothManager::showBluetoothSettings()
{
    Q_D(BluetoothManager);

    d->m_controlcenterInter->ShowModule(BluetoothPage);
}

void BluetoothManager::sendFiles(const QString &id, const QStringList &filePath, const QString &senderToken)
{
    Q_D(BluetoothManager);

    // /org/bluez/hci0/dev_90_63_3B_DA_5A_4C  --》  90:63:3B:DA:5A:4C
    QString deviceAddress = id;
    deviceAddress.remove(QRegularExpression("/org/bluez/hci[0-9]*/dev_")).replace("_", ":");

    QFuture<QPair<QString, QString>> future = QtConcurrent::run([this, deviceAddress, filePath] {
       Q_D(BluetoothManager);
       QDBusPendingReply<QDBusObjectPath> reply = d->m_bluetoothInter->SendFiles(deviceAddress, filePath);
       reply.waitForFinished();
       return qMakePair<QString, QString>(reply.value().path(), reply.error().message());
    });

    if (d->m_watcher) {
        if (d->m_watcher->isRunning())
            d->m_watcher->future().cancel();
        delete d->m_watcher;
        d->m_watcher = nullptr;
    }

    // 此处 watcher 在 run 完成之后会 delete，但无法在传输对话框关闭后立即 delete
    d->m_watcher = new QFutureWatcher<QPair<QString, QString>>();
    d->m_watcher->setFuture(future);
    connect(d->m_watcher, &QFutureWatcher<QString>::finished, this, [d, senderToken, this]{
        emit transferEstablishFinish(d->m_watcher->result().first, d->m_watcher->result().second, senderToken);
        delete d->m_watcher;
        d->m_watcher = nullptr;
   });
}

bool BluetoothManager::cancelTransfer(const QString &sessionPath)
{
    Q_D(BluetoothManager);
    d->m_bluetoothInter->CancelTransferSession(QDBusObjectPath(sessionPath));
    qDebug() << sessionPath;
    return true;
}

bool BluetoothManager::canSendBluetoothRequest()
{
    Q_D(BluetoothManager);
    return d->m_bluetoothInter->transportable();
}
