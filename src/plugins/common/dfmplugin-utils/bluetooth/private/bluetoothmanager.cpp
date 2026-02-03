// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetoothmanager_p.h"
#include "bluetoothmodel.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QFileInfo>

#include <limits.h>

#ifdef COMPILE_ON_V20
#    define BluetoothService "com.deepin.daemon.Bluetooth"
#    define BluetoothPath "/com/deepin/daemon/Bluetooth"
#    define BluetoothInterface "com.deepin.daemon.Bluetooth"
#    define ControlcenterService "com.deepin.dde.ControlCenter"
#    define ControlcenterPath "/com/deepin/dde/ControlCenter"
#    define ControlcenterInterface "com.deepin.dde.ControlCenter"
#else
#    define BluetoothService "org.deepin.dde.Bluetooth1"
#    define BluetoothPath "/org/deepin/dde/Bluetooth1"
#    define BluetoothInterface "org.deepin.dde.Bluetooth1"
#    define ControlcenterService "org.deepin.dde.ControlCenter1"
#    define ControlcenterPath "/org/deepin/dde/ControlCenter1"
#    define ControlcenterInterface "org.deepin.dde.ControlCenter1"
#endif

#define BluetoothPage "bluetooth"

using namespace dfmplugin_utils;

BluetoothManagerPrivate::BluetoothManagerPrivate(BluetoothManager *qq)
    : QObject(qq),
      q_ptr(qq),
      model(new BluetoothModel(qq))
{
    initInterface();
    initConnects();
}

BluetoothManagerPrivate::~BluetoothManagerPrivate()
{
    if (bluetoothInter) {
        delete bluetoothInter;
        bluetoothInter = nullptr;
    }

    if (watcher) {
        if (watcher->isRunning()) {
            watcher->future().cancel();
        }
        delete watcher;
        watcher = nullptr;
    }
}

/**
 * @brief 解析蓝牙设备, 获取适配器和设备信息
 * @param req
 */
void BluetoothManagerPrivate::resolve(const QDBusReply<QString> &req)
{
    Q_Q(BluetoothManager);
    const QString replyStr = req.value();
    fmInfo() << replyStr;
    static int maxRetry = 3;
    // GetAdapter method, if there has no adapter, the method returns an emtpy json [], but if the method is not ready,
    // returns a null string. if null, retry
    if (replyStr.isEmpty() && maxRetry > 0) {
        fmInfo() << "retry to get bluetooth adapters..." << maxRetry;
        QTimer::singleShot(500, q, [q] {
            q->refresh();
        });
        maxRetry--;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(replyStr.toUtf8());
    QJsonArray arr = doc.array();
    for (QJsonValue val : arr) {
        BluetoothAdapter *adapter = new BluetoothAdapter(model);
        inflateAdapter(adapter, val.toObject());
        model->addAdapter(adapter);
    }
}

void BluetoothManagerPrivate::initInterface()
{
    Q_Q(BluetoothManager);
    // initialize dbus interface
    if (bluetoothInter)
        delete bluetoothInter;
    bluetoothInter = new QDBusInterface(BluetoothService, BluetoothPath, BluetoothInterface,
                                        QDBusConnection::sessionBus(), q);
    bluetoothInter->setTimeout(2000);
}

void BluetoothManagerPrivate::initConnects()
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(BluetoothService, QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForRegistration, this);
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, [=](const QString &service) {
        if (service == BluetoothService) {
            fmDebug() << "bluetooth: service registered. ";
            this->onServiceValidChanged(true);
        }
    });

    connectBluetoothDBusSignals("AdapterAdded", SLOT(onAdapterAdded(const QString &)));
    connectBluetoothDBusSignals("AdapterRemoved", SLOT(onAdapterRemoved(const QString &)));
    connectBluetoothDBusSignals("AdapterPropertiesChanged", SLOT(onAdapterPropertiesChanged(const QString &)));
    connectBluetoothDBusSignals("DeviceAdded", SLOT(onDeviceAdded(const QString &)));
    connectBluetoothDBusSignals("DeviceRemoved", SLOT(onDeviceRemoved(const QString &)));
    connectBluetoothDBusSignals("DevicePropertiesChanged", SLOT(onDevicePropertiesChanged(const QString &)));
    connectBluetoothDBusSignals("TransferCreated", SLOT(onTransferCreated(const QString &, const QDBusObjectPath &, const QDBusObjectPath &)));
    connectBluetoothDBusSignals("TransferRemoved", SLOT(onTransferRemoved(const QString &, const QDBusObjectPath &, const QDBusObjectPath &, bool)));
    connectBluetoothDBusSignals("ObexSessionCreated", SLOT(onObexSessionCreated(const QDBusObjectPath &)));
    connectBluetoothDBusSignals("ObexSessionRemoved", SLOT(onObexSessionRemoved(const QDBusObjectPath &)));
    connectBluetoothDBusSignals("ObexSessionProgress", SLOT(onObexSessionProgress(const QDBusObjectPath &, qulonglong, qulonglong, int)));
    connectBluetoothDBusSignals("TransferFailed", SLOT(onTransferFailed(const QString &, const QDBusObjectPath &, const QString &)));
}

bool BluetoothManagerPrivate::connectBluetoothDBusSignals(const QString &signal, const char *slot)
{
    return QDBusConnection::sessionBus().connect(BluetoothService, BluetoothPath, BluetoothInterface, signal, this, slot);
}

/**
 * @brief 获取适配器信息
 * @param adapter
 * @param adapterObj
 */
void BluetoothManagerPrivate::inflateAdapter(BluetoothAdapter *adapter, const QJsonObject &adapterObj)
{
    Q_Q(BluetoothManager);

    const QString path = adapterObj["Path"].toString();
    const QString alias = adapterObj["Alias"].toString();
    const bool powered = adapterObj["Powered"].toBool();
    fmDebug() << "resolve adapter path:" << path;

    adapter->setId(path);
    adapter->setName(alias);
    adapter->setPowered(powered);

    QPointer<BluetoothAdapter> adapterPointer(adapter);

    // 异步获取适配器的所有设备
    QDBusObjectPath dPath(path);
    QDBusPendingCall call = getBluetoothDevices(dPath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, q, [this, watcher, adapterPointer, call] {
        if (!adapterPointer) {
            fmDebug() << "adapterPointer released!";
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
            for (const BluetoothDevice *device : adapter->getDevices()) {
                if (!tmpList.contains(device->getId())) {
                    adapter->removeDevice(device->getId());

                    BluetoothDevice *target = const_cast<BluetoothDevice *>(device);
                    if (target) {
                        target->deleteLater();
                    }
                }
            }
        } else {
            fmWarning() << call.error().message();
        }

        watcher->deleteLater();
    });
}

/**
 * @brief 获取设备信息
 * @param device
 * @param deviceObj
 */
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

QDBusPendingCall BluetoothManagerPrivate::getBluetoothDevices(const QDBusObjectPath &adapter)
{
    return bluetoothInter->asyncCallWithArgumentList("GetDevices", { QVariant::fromValue(adapter) });
}

QDBusPendingCall BluetoothManagerPrivate::getBluetoothAdapters()
{
    return bluetoothInter->asyncCall("GetAdapters");
}

QDBusPendingReply<QDBusObjectPath> BluetoothManagerPrivate::sendFiles(const QString &device, const QStringList &files)
{
    return bluetoothInter->asyncCallWithArgumentList("SendFiles", { QVariant::fromValue(device), QVariant::fromValue(files) });
}

void BluetoothManagerPrivate::cancelTransferSession(const QDBusObjectPath &sessionPath)
{
    bluetoothInter->asyncCallWithArgumentList("CancelTransferSession", { QVariant::fromValue(sessionPath) });
}

void BluetoothManagerPrivate::onServiceValidChanged(bool valid)
{
    Q_Q(BluetoothManager);
    if (valid) {
        fmInfo() << "bluetooth service is valid now...";
        initInterface();
        QTimer::singleShot(1000, q, [q] { q->refresh(); });
    }
}

void BluetoothManagerPrivate::onAdapterAdded(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();

    BluetoothAdapter *adapter = new BluetoothAdapter(model);
    inflateAdapter(adapter, obj);
    model->addAdapter(adapter);
}

void BluetoothManagerPrivate::onAdapterRemoved(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    const QString id = obj["Path"].toString();

    const BluetoothAdapter *result = model->removeAdapater(id);
    BluetoothAdapter *adapter = const_cast<BluetoothAdapter *>(result);
    if (adapter) {
        adapter->deleteLater();
    }
}

void BluetoothManagerPrivate::onAdapterPropertiesChanged(const QString &json)
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    const QJsonObject obj = doc.object();
    const QString id = obj["Path"].toString();

    BluetoothAdapter *adapter = const_cast<BluetoothAdapter *>(model->adapterById(id));
    if (adapter) {
        inflateAdapter(adapter, obj);
    }
}

void BluetoothManagerPrivate::onDeviceAdded(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    const QString adapterId = obj["AdapterPath"].toString();
    const QString id = obj["Path"].toString();

    const BluetoothAdapter *result = model->adapterById(adapterId);
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
}

void BluetoothManagerPrivate::onDeviceRemoved(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    const QString adapterId = obj["AdapterPath"].toString();
    const QString id = obj["Path"].toString();

    const BluetoothAdapter *result = model->adapterById(adapterId);
    BluetoothAdapter *adapter = const_cast<BluetoothAdapter *>(result);
    if (adapter) {
        adapter->removeDevice(id);
    }
}

void BluetoothManagerPrivate::onDevicePropertiesChanged(const QString &json)
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    const QJsonObject obj = doc.object();
    const QString id = obj["Path"].toString();
    for (const BluetoothAdapter *adapter : model->getAdapters()) {
        BluetoothDevice *device = const_cast<BluetoothDevice *>(adapter->deviceById(id));
        if (device) {
            inflateDevice(device, obj);
        }
    }
}

void BluetoothManagerPrivate::onTransferCreated(const QString &file, const QDBusObjectPath &transferPath, const QDBusObjectPath &sessionPath)
{
    fmDebug() << file << transferPath.path() << sessionPath.path();
}

void BluetoothManagerPrivate::onTransferRemoved(const QString &file, const QDBusObjectPath &transferPath, const QDBusObjectPath &sessionPath, bool done)
{
    Q_UNUSED(transferPath)
    Q_Q(BluetoothManager);
    QString sessionPathStr = sessionPath.path();

    if (!done) {
        // Extract filename from file path using string operations
        int lastSlash = file.lastIndexOf('/');
        QString filename = (lastSlash >= 0) ? file.mid(lastSlash + 1) : file;

        QByteArray utf8Name = filename.toUtf8();
        bool isTooLong = utf8Name.size() > NAME_MAX;

        longFilenameFailures[sessionPathStr] = isTooLong;
        fmWarning() << "bluetooth TransferRemoved: failed by long filename:" << isTooLong 
                    << " namelen=" << utf8Name.size() << " session=" << sessionPathStr;
        Q_EMIT q->transferCancledByRemote(sessionPathStr);
    } else {
        longFilenameFailures.remove(sessionPathStr);
        Q_EMIT q->fileTransferFinished(sessionPathStr, file);
    }
}

void BluetoothManagerPrivate::onObexSessionCreated(const QDBusObjectPath &sessionPath)
{
    fmDebug() << sessionPath.path();
}

void BluetoothManagerPrivate::onObexSessionRemoved(const QDBusObjectPath &sessionPath)
{
    QString sessionPathStr = sessionPath.path();
    longFilenameFailures.remove(sessionPathStr);
    fmDebug() << sessionPathStr;
}

void BluetoothManagerPrivate::onObexSessionProgress(const QDBusObjectPath &sessionPath, qulonglong totalSize, qulonglong transferred, int currentIndex)
{
    Q_Q(BluetoothManager);
    Q_EMIT q->transferProgressUpdated(sessionPath.path(), totalSize, transferred, currentIndex);
}

void BluetoothManagerPrivate::onTransferFailed(const QString &file, const QDBusObjectPath &sessionPath, const QString &errInfo)
{
    Q_Q(BluetoothManager);
    Q_EMIT q->transferFailed(sessionPath.path(), file, errInfo);
}

/**
 * @brief This is BluetoothManager implement
 */
BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent),
      d_ptr(new BluetoothManagerPrivate(this))
{
    refresh();

    connect(d_ptr->model, &BluetoothModel::adapterAdded, this, &BluetoothManager::adapterAdded);
    connect(d_ptr->model, &BluetoothModel::adapterRemoved, this, &BluetoothManager::adapterRemoved);
}

BluetoothManager *BluetoothManager::instance()
{
    static BluetoothManager bluetooth;
    return &bluetooth;
}

/**
 * @brief 异步接口：调用刷新蓝牙模块，初始化数据
 */
void BluetoothManager::refresh()
{
    Q_D(BluetoothManager);
    if (!d->bluetoothInter || !d->bluetoothInter->isValid()) {
        fmCritical() << "bluetooth interface is not valid!!!" << d->bluetoothInter->lastError();
        return;
    }

    // 获取蓝牙设备
    QDBusPendingCall call = d->getBluetoothAdapters();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
        if (!call.isError()) {
            QDBusReply<QString> reply = call.reply();
            d->resolve(reply);
        } else {
            fmWarning() << call.error().message();
        }
        watcher->deleteLater();
    });
}

bool BluetoothManager::hasAdapter()
{
    return getAdapters().count() > 0;
}

bool BluetoothManager::bluetoothSendEnable()
{
    Q_D(BluetoothManager);

    if (!d->bluetoothInter->isValid()) {
        fmWarning() << "bluetooth interface is not valid";
        return false;
    }
    auto canSendFile = d->bluetoothInter->property("CanSendFile");
    if (!canSendFile.isValid()) {
        fmWarning() << "bluetooth interface has no 'CanSendFile' property";
        return false;
    }
    return canSendFile.toBool();
}

QMap<QString, const BluetoothAdapter *> BluetoothManager::getAdapters() const
{
    return d_ptr->model->getAdapters();
}

/**
 * @brief 打开控制中心的'蓝牙'界面
 */
void BluetoothManager::showBluetoothSettings()
{
    QDBusInterface controCenter(ControlcenterService, ControlcenterPath, ControlcenterInterface,
                                QDBusConnection::sessionBus(), this);
    controCenter.asyncCallWithArgumentList("ShowModule", { QVariant::fromValue(QString(BluetoothPage)) });
}

/**
 * @brief sendFiles     向设备发送文件
 * @param id            蓝牙设备 ID
 * @param filePath      文件路径列表
 * @param senderToken   发起该次传输请求的对话框唯一标识符
 */
void BluetoothManager::sendFiles(const QString &id, const QStringList &filePath, const QString &senderToken)
{
    Q_D(BluetoothManager);

    // /org/bluez/hci0/dev_90_63_3B_DA_5A_4C  --》  90:63:3B:DA:5A:4C
    QString deviceAddress = id;
    deviceAddress.remove(QRegularExpression("/org/bluez/hci[0-9]*/dev_")).replace("_", ":");

    QFuture<QPair<QString, QString>> future = QtConcurrent::run([this, deviceAddress, filePath] {
        Q_D(BluetoothManager);
        QDBusPendingReply<QDBusObjectPath> reply = d->sendFiles(deviceAddress, filePath);
        reply.waitForFinished();
        return qMakePair<QString, QString>(reply.value().path(), reply.error().message());
    });

    if (d->watcher) {
        if (d->watcher->isRunning())
            d->watcher->future().cancel();
        delete d->watcher;
        d->watcher = nullptr;
    }

    // 此处 watcher 在 run 完成之后会 delete，但无法在传输对话框关闭后立即 delete
    d->watcher = new QFutureWatcher<QPair<QString, QString>>();
    d->watcher->setFuture(future);
    connect(d->watcher, &QFutureWatcher<QString>::finished, this, [d, senderToken, this] {
        emit transferEstablishFinish(d->watcher->result().first, d->watcher->result().second, senderToken);
        delete d->watcher;
        d->watcher = nullptr;
    });
}

/**
 * @brief cancleTransfer 取消某个传输会话
 * @param sessionPath
 * @return
 */
bool BluetoothManager::cancelTransfer(const QString &sessionPath)
{
    Q_D(BluetoothManager);
    d->cancelTransferSession(QDBusObjectPath(sessionPath));
    fmDebug() << sessionPath;
    return true;
}

bool BluetoothManager::canSendBluetoothRequest()
{
    Q_D(BluetoothManager);
    auto transportable = d->bluetoothInter->property("Transportable");
    return transportable.isValid() ? transportable.toBool() : true;
}

bool BluetoothManager::isLongFilenameFailure(const QString &sessionPath) const
{
    Q_D(const BluetoothManager);
    return d->longFilenameFailures.value(sessionPath, false);
}
