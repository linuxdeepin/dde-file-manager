/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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

#include "accesscontrolmanager.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QList>
#include <QJsonDocument>
#include <QJsonParseError>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <ddiskdevice.h>
#include <ddiskmanager.h>
#include <dblockdevice.h>

#include "dfilesystemwatcher.h"

#include "app/policykithelper.h"
#include "dbusservice/dbusadaptor/accesscontrol_adaptor.h"

#define KEY_INVOKER     "invoker"
#define KEY_TYPE        "type"
#define KEY_DEVICE      "device"
#define KEY_POLICY      "policy"
#define KEY_TSTAMP      "timestamp"
#define KEY_GLOBAL      "global"

#define KEY_ERRSTR      "errstr"
#define KEY_ERRNO       "errno"

#define POLICYTYPE      "policytype"
#define VAULTHIDESTATE  "vaulthidestate"
#define POLICYSTATE     "policystate"

#define TYPE_INVALID    0x00
#define TYPE_BLOCK      0x01
#define TYPE_OPTICAL    0x02
#define TYPE_PROTOCOL   0x04

#define POLICY_DISABLE    0
#define POLICY_RONLY      1
#define POLICY_RW         2

#define MAX_RETRY       5

QString AccessControlManager::ObjectPath = "/com/deepin/filemanager/daemon/AccessControlManager";
QString AccessControlManager::PolicyKitActionId = "com.deepin.filemanager.daemon.AccessControlManager";

AccessControlManager::AccessControlManager(QObject *parent)
    : QObject(parent)
    , QDBusContext(),
      m_watcher(new DFileSystemWatcher(this))
{
    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("AccessControlManager Register Object Failed.");
    }
    new AccessControlAdaptor(this); // 注册接口，无需自己释放内存
    m_diskMnanager = new DDiskManager(this);
    m_diskMnanager->setWatchChanges(true);

    m_whiteProcess << "/usr/bin/dmcg" << "/usr/bin/dde-file-manager";
    m_configPath = "/etc/deepin/devAccessConfig.json";
    m_vaultConfigPath = "/etc/deepin/vaultAccessConfig.json";
    loadPolicy();
    loadVaultPolicy();
    changeMountedOnInit();

    m_errMsg.insert(NoError, "");
    m_errMsg.insert(InvalidArgs, tr("Invalid args"));
    m_errMsg.insert(InvalidInvoker, tr("Invalid invoker"));

    m_watcher->addPath("/home");
    onFileCreated("/home", "root");
    initConnect();
}

AccessControlManager::~AccessControlManager()
{

}

void AccessControlManager::initConnect()
{
    connect(m_diskMnanager, &DDiskManager::mountAdded, this, &AccessControlManager::chmodMountpoints);
    connect(m_watcher, &DFileSystemWatcher::fileCreated, this, &AccessControlManager::onFileCreated);
    connect(m_diskMnanager, &DDiskManager::diskDeviceAdded, this, &AccessControlManager::disconnOpticalDev);
}

bool AccessControlManager::checkAuthentication()
{
    bool ret = false;
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if(c.isConnected()) {
       pid = c.interface()->servicePid(message().service()).value();
    }

    if (pid){
        ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitActionId, pid);
    }

    if (!ret) {
        qWarning() << "Authentication failed !!";
        qWarning() << "failed pid: " << pid;
        qWarning() << "failed policy id:" << PolicyKitActionId;
    }
    return ret;
}

QString AccessControlManager::SetAccessPolicy(const QVariantMap &policy)
{
    QVariantMap sigInfo;
    // 0. 接口访问权限
    uint invokerPid = connection().interface()->servicePid(message().service()).value();
    QString invokerPath;
    if (!isValidInvoker(invokerPid, invokerPath)) {
        sigInfo = policy;
        sigInfo.insert(KEY_ERRNO, InvalidInvoker);
        sigInfo.insert(KEY_ERRSTR, m_errMsg.value(InvalidInvoker));
        emit AccessPolicySetFinished(sigInfo);
        qDebug() << invokerPath << " is not allowed to invoke this function";
        return invokerPath + " is not allowed";
    }

    // 1. 校验策略有效性
    if (!isValidPolicy(policy, invokerPath)) {
        sigInfo = policy;
        sigInfo.insert(KEY_ERRNO, InvalidArgs);
        sigInfo.insert(KEY_ERRSTR, m_errMsg.value(InvalidArgs));
        emit AccessPolicySetFinished(sigInfo);
        qDebug() << "policy is not valid";
        return QString("policy is not valid");
    }

    // 2. 写入配置文件
    savePolicy(policy);

    // 2.5 加载最新的策略到内存
    loadPolicy();

    // 2.5.5 发送信号通知策略已完成修改
    sigInfo.insert(KEY_INVOKER, invokerPath);
    sigInfo.insert(KEY_TYPE, policy.value(KEY_TYPE));
    sigInfo.insert(KEY_POLICY, policy.value(KEY_POLICY));
    sigInfo.insert(KEY_ERRNO, NoError);
    sigInfo.insert(KEY_ERRSTR, "");
    emit AccessPolicySetFinished(sigInfo);
    QVariantList sigInfos;
    QMapIterator<int, QPair<QString, int>> iter(m_globalPolicies);
    while (iter.hasNext()) {
        iter.next();
        QVariantMap item;
        item.insert(KEY_TYPE, iter.key());
        item.insert(KEY_POLICY, iter.value().second);
        sigInfos << item;
    }
    emit DeviceAccessPolicyChanged(sigInfos);

    // 3. 改变已挂载设备的访问权限；现阶段不接入此功能；
//    changeMountedPolicy(policy);
    return "OK";
}

QVariantList AccessControlManager::QueryAccessPolicy()
{
    QVariantList ret;
    QVariantMap item;
    QMapIterator<int, QPair<QString, int>> iter(m_globalPolicies);
    while (iter.hasNext()) {
        iter.next();
        item.clear();
        item.insert(KEY_TYPE, iter.key());
        item.insert(KEY_POLICY, iter.value().second);
        item.insert(KEY_INVOKER, iter.value().first);
        ret << item;
    }
    return ret;
}

/**
 * @brief AccessControlManager::SetVaultAccessPolicy
 * @param policy POLICYTYPE 1表示保险箱, VAULTHIDESTATE 1表示隐藏保险箱 2表示显示保险箱, POLICYSTATE 1表示策略执行 2表示策略不执行
 * @return 返回执行结果
 */
QString AccessControlManager::SetVaultAccessPolicy(const QVariantMap &policy)
{
    QVariantMap sigInfo;
    // 0. 接口访问权限
    uint invokerPid = connection().interface()->servicePid(message().service()).value();
    QString invokerPath;
    if (!isValidInvoker(invokerPid, invokerPath)) {
        sigInfo = policy;
        sigInfo.insert(KEY_ERRNO, InvalidInvoker);
        sigInfo.insert(KEY_ERRSTR, m_errMsg.value(InvalidInvoker));
        emit AccessPolicySetFinished(sigInfo);
        qInfo() << invokerPath << " is not allowed to invoke this function";
        return invokerPath + " is not allowed";
    }

    // 1. 校验策略有效性
    if (!isValidVaultPolicy(policy)) {
        sigInfo = policy;
        sigInfo.insert(KEY_ERRNO, InvalidArgs);
        sigInfo.insert(KEY_ERRSTR, m_errMsg.value(InvalidArgs));
        emit AccessPolicySetFinished(sigInfo);
        qDebug() << "policy is not valid";
        return QString("policy is not valid");
    }

    saveVaultPolicy(policy);

    loadVaultPolicy();

    if(m_VaultHidePolicies.isEmpty())
        return QString("");

    // 2.5.5 发送信号通知策略已完成修改
    sigInfo.insert(POLICYTYPE, policy.value(POLICYTYPE));
    sigInfo.insert(VAULTHIDESTATE, policy.value(VAULTHIDESTATE));
    sigInfo.insert(POLICYSTATE, policy.value(POLICYSTATE));
    sigInfo.insert(KEY_ERRNO, NoError);
    sigInfo.insert(KEY_ERRSTR, "");
    emit AccessPolicySetFinished(sigInfo);

    emit AccessVaultPolicyNotify();

    return QString("OK");
}

QVariantList AccessControlManager::QueryVaultAccessPolicy()
{
    QVariantList ret;
    QVariantMap item;
    QMapIterator<QString, int> iter(m_VaultHidePolicies);
    while (iter.hasNext()) {
        iter.next();
        item.insert(iter.key(), iter.value());
    }
    ret << QVariant::fromValue(item);
    return ret;
}

int AccessControlManager::QueryVaultAccessPolicyVisible()
{
    if(m_VaultHidePolicies.value(POLICYSTATE) == 1)
        return m_VaultHidePolicies.value(VAULTHIDESTATE);
    else
        return 0;
}

QString AccessControlManager::FileManagerReply(int policystate)
{
    QVariantList listMap = QueryVaultAccessPolicy();
    QVariantMap map = listMap.at(0).toMap();
    map.insert(POLICYSTATE, policystate);
    SetVaultAccessPolicy(map);
    return "OK";
}

/**
 * @brief 设备挂载后，为挂载路径添加写权限
 *        一些设备被 root 挂载，对其他用户没写权限，导致无法正常操作（相关 bug-60788）
 * @param blockDevicePath
 * @param mountPoint
 */
void AccessControlManager::chmodMountpoints(const QString &blockDevicePath, const QByteArray &mountPoint)
{
    qDebug() << "blockDevicePath: " << blockDevicePath;
    QScopedPointer<DBlockDevice> blk(m_diskMnanager->createBlockDevice(blockDevicePath));
    if (m_globalPolicies.contains(TYPE_BLOCK) && blk) {
        QString devDesc = blk->device();
        int mode = accessMode(mountPoint);
        QString source = m_globalPolicies.value(TYPE_BLOCK).first;
        int policy = m_globalPolicies.value(TYPE_BLOCK).second;
        QString fs = blk->idType();
        if (mode != policy) {
            if (policy == POLICY_DISABLE) {
                // unmount
            } else {
                // remount
                QtConcurrent::run([devDesc, mountPoint, fs, policy, source](){
                    int ret = mount(devDesc.toLocal8Bit().data(),
                                    mountPoint.data(),
                                    fs.toLocal8Bit().data(),
                                    MS_REMOUNT | (policy == POLICY_RONLY ? MS_RDONLY : 0),
                                    nullptr);
                    if (ret == 0)
                        qDebug() << "remount with policy " << policy << " from " << source;
                    else
                        qDebug() << "remount with policy " << policy << " failed, errno: " << errno << ", errstr: " << strerror(errno);
                });
            }
        }
        if (policy != POLICY_RW) // 当格式化后，挂载点属组变为了 root 且权限变为了 755，其他用户变得无权限访问设备，因此这里在控制权限为 RW 的时候，继续执行后续变更挂载点权限的代码；
            return;
    }

    qInfo() << "chmod ==>" << mountPoint;
    struct stat fileStat;
    stat(mountPoint.data(), &fileStat);
    chmod(mountPoint.data(), (fileStat.st_mode | S_IWUSR | S_IWGRP | S_IWOTH));
}

void AccessControlManager::disconnOpticalDev(const QString &drivePath)
{
    QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(drivePath));
    if (!drv || !drv->canPowerOff() || drv->connectionBus() != "usb") // 不能断电的通常为内置光驱
        return;
    // note: drv->optical 设备接入时此字段为 false，不能用于判断是否是光驱
    const QStringList &compatibility = drv->mediaCompatibility();
    if (compatibility.count() == 0 || !compatibility.first().contains("optical"))
        return;
    if (!m_globalPolicies.contains(TYPE_OPTICAL)) {
        return;
    }

    int policy = m_globalPolicies.value(TYPE_OPTICAL).second;
    qDebug() << drv << drv->optical() << m_globalPolicies.contains(TYPE_OPTICAL) << policy;
    if (policy == POLICY_DISABLE) {
        QtConcurrent::run([drv](){
            int retry = 0;
            do {
                retry++;
                drv->powerOff({});
                QDBusError err = drv->lastError();
                if (err.type() == QDBusError::NoError || retry == MAX_RETRY)
                    break;
                else
                    qDebug() << "poweroff device: " << drv->lastError().message();
                QThread::msleep(500);
            } while (1);
        });
    }
}

bool AccessControlManager::isValidPolicy(const QVariantMap &policy, const QString &realInvoker)
{
    // invoker must not be empty
    // type must in (0, 7]
    // policy must in [0, 2]
    // device is optional
    return policy.contains(KEY_INVOKER) && !policy.value(KEY_INVOKER).toString().isEmpty()
            && policy.contains(KEY_TYPE) && policy.value(KEY_TYPE).toInt() > TYPE_INVALID && policy.value(KEY_TYPE).toInt() <= (TYPE_BLOCK | TYPE_OPTICAL | TYPE_PROTOCOL)
            && policy.contains(KEY_POLICY) && policy.value(KEY_POLICY).toInt() >= POLICY_DISABLE && policy.value(KEY_POLICY).toInt() <= POLICY_RW
            && policy.value(KEY_INVOKER).toString() == realInvoker;
}

void AccessControlManager::savePolicy(const QVariantMap &policy)
{
    // 1. if file does not exist then create it
    QFile config(m_configPath);
    if (!config.open(QIODevice::ReadWrite)) {
        qDebug() << "config open failed";
        return;
    }
    config.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    int inGlobal = (!policy.contains(KEY_DEVICE) || policy.value(KEY_DEVICE).toString().isEmpty())
            ? 1 : 0;
    int inType = policy.value(KEY_TYPE).toInt();
    int inPolicy = policy.value(KEY_POLICY).toInt();
    QString inDevice = inGlobal ? "" : policy.value(KEY_DEVICE).toString();
    QString inInvoker = policy.value(KEY_INVOKER).toString();

    // 2. append/replace config to configFile
    bool foundExist = false;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    QJsonArray newArr;
    if (doc.isArray()) {
       QJsonArray arr = doc.array();
       foreach (auto obj, arr) {
           if (!obj.isObject())
               continue;
           QJsonObject objInfo = obj.toObject();
           int global = objInfo.contains(KEY_GLOBAL) ? objInfo.value(KEY_GLOBAL).toInt() : 0;
           QString src = objInfo.contains(KEY_INVOKER) ? objInfo.value(KEY_INVOKER).toString() : "";
           int type = objInfo.contains(KEY_TYPE) ? objInfo.value(KEY_TYPE).toInt() : 0;
           QString timestamp = objInfo.contains(KEY_TSTAMP) ? objInfo.value(KEY_TSTAMP).toString() : "";
           QString dev = objInfo.contains(KEY_DEVICE) ? objInfo.value(KEY_DEVICE).toString() : "";
           QString invoker = objInfo.contains(KEY_INVOKER) ? objInfo.value(KEY_INVOKER).toString() : "";

           if (inGlobal == global && inType == type && inDevice == dev && inInvoker == invoker) {
               foundExist = true;
               objInfo.insert(KEY_POLICY, inPolicy);
               objInfo.insert(KEY_TSTAMP, QString::number(QDateTime::currentSecsSinceEpoch()));
               qDebug() << "found exist policy, just updtae it";
           }

           newArr.append(objInfo);
       }
    }
    if (!foundExist) {
        QJsonObject obj;
        obj.insert(KEY_GLOBAL, inGlobal ? 1: 0);
        obj.insert(KEY_INVOKER, inInvoker);
        obj.insert(KEY_TYPE, inType);
        obj.insert(KEY_POLICY, inPolicy);
        obj.insert(KEY_TSTAMP, QString::number(QDateTime::currentSecsSinceEpoch()));
        if (inGlobal == 0)
            obj.insert(KEY_DEVICE, inDevice);
        newArr.append(obj);
        qDebug() << "append new policy";
    }
    doc.setArray(newArr);
    config.close();
    config.open(QIODevice::Truncate | QIODevice::ReadWrite); // overwrite the config file
    config.write(doc.toJson());
    config.close();
}

void AccessControlManager::changeMountedPolicy(const QVariantMap &policy)
{
    int type = policy.value(KEY_TYPE).toInt();
    int mode = policy.value(KEY_POLICY).toInt();
    QString device = policy.value(KEY_DEVICE).toString();
    qDebug() << "type = " << type << "\t" << "policy = " << mode;
    if (type & TYPE_BLOCK) {
        changeMountedBlock(mode, device);
    }
    if (type & TYPE_OPTICAL) {
        changeMountedOptical(mode, device);
    }
    if (type & TYPE_PROTOCOL) {
        changeMountedProtocol(mode, device);
    }
}

bool AccessControlManager::isValidInvoker(uint pid, QString &invokerPath)
{
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    if (!f.exists())
        return false;
    invokerPath = f.canonicalFilePath();
    return m_whiteProcess.contains(invokerPath);
}

void AccessControlManager::changeMountedBlock(int mode, const QString &device)
{
    Q_UNUSED(device)

    const QStringList &blocks = m_diskMnanager->blockDevices({});
    QList<MountArgs> waitToHandle;
    foreach (auto block, blocks) {
        QScopedPointer<DBlockDevice> blkDev(DDiskManager::createBlockDevice(block));
        if (!blkDev || !blkDev->hasFileSystem() || blkDev->mountPoints().isEmpty())
            continue;

        QScopedPointer<DDiskDevice> drvDev(DDiskManager::createDiskDevice(blkDev->drive()));
        if (!drvDev || !drvDev->removable() || drvDev->optical())
            continue;

        QString mps = blkDev->mountPoints().first();
        // 0. 检查是否是目标设备

        // 1. 检查设备是否在白名单内

        // 2. 检查挂载点权限是否与策略一致，不一致则需要更改
        int mountedMode = accessMode(mps);
        if (mountedMode == mode)
            continue;

        // 3. 需要重载或卸载
        const QString &devDesc = blkDev->device(); // 设备描述符
        MountArgs args;
        args.devDesc = devDesc;
        args.mountPoint = mps;
        args.fileSystem = blkDev->idType();
        waitToHandle.append(args);
    }

    // 4. 开启线程处理重载/卸载任务
    if (waitToHandle.count() > 0) {
        QtConcurrent::run([waitToHandle, mode](){
            foreach(auto dev, waitToHandle) {
                if (mode == 0) { // unmount
                    int ret = umount(dev.mountPoint.toLocal8Bit().data());
                } else {    // remount
                    int ret = mount(dev.devDesc.toLocal8Bit().data(),
                                    dev.mountPoint.toLocal8Bit().data(),
                                    dev.fileSystem.toLocal8Bit().data(),
                                    MS_REMOUNT | (mode == POLICY_RONLY ? MS_RDONLY : 0),
                                    nullptr);
                    if (ret < 0)
                        qDebug() << "remount " << dev.devDesc << " failed: " << errno << ": " << strerror(errno);
                }
            }
        });
    }
}

void AccessControlManager::changeMountedOptical(int mode, const QString &device)
{
    Q_UNUSED(device)
    // 只能主动关闭，不能主动打开；光驱只负责 DISABLE / RW
    if (mode != POLICY_DISABLE)
        return;
    const QStringList &blocks = DDiskManager::blockDevices({});
    const QStringList &disks = m_diskMnanager->diskDevices();
    QMap<QString, QSharedPointer<DDiskDevice>> opticalDrv;
    foreach (auto drvPath, disks) { // 获取系统所有 USB 光驱设备对象
        QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(drvPath));
        if (!drv)
            continue;
        const QStringList &compatibility = drv->mediaCompatibility();
        if (compatibility.count() == 0 || !compatibility.first().contains("optical") || drv->connectionBus() != "usb")
            continue;
        opticalDrv.insert(drvPath, drv);
    }

    QtConcurrent::run([blocks, opticalDrv]{
        foreach (auto blkPath, blocks) {
            QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blkPath));
            if (!blk || !opticalDrv.contains(blk->drive()))
                continue;

            // unmount first
            if (blk->mountPoints().count() > 0) {
                blk->unmount({});
                if (blk->lastError().type() != QDBusError::NoError) {
                    qDebug() << "Error occured while unmount optical device: " << blk->lastError().message();
                    continue;
                }
            }
            QThread::msleep(500);
            // poweroff it
            QSharedPointer<DDiskDevice> drv = opticalDrv.value(blk->drive());
            if (drv) {
                int retry = 0;
                do {
                    retry++;
                    drv->powerOff({});
                    QDBusError err = drv->lastError();
                    if (err.type() == QDBusError::NoError || retry == MAX_RETRY)
                        break;
                    else
                        qDebug() << "Error occured while poweroff optical device: " << drv->lastError().message();
                    QThread::msleep(500);
                } while (1);
            }
        }
    });
}

void AccessControlManager::changeMountedProtocol(int mode, const QString &device)
{
    Q_UNUSED(mode)
    Q_UNUSED(device)
}

void AccessControlManager::changeMountedOnInit()
{
    qDebug() << "start change access on init...";
    if (m_globalPolicies.contains(TYPE_BLOCK))
        changeMountedBlock(m_globalPolicies.value(TYPE_BLOCK).second, "");
    if (m_globalPolicies.contains(TYPE_OPTICAL))
        changeMountedOptical(m_globalPolicies.value(TYPE_OPTICAL).second, "");
    if (m_globalPolicies.contains(TYPE_PROTOCOL))
        changeMountedProtocol(m_globalPolicies.value(TYPE_PROTOCOL).second, "");
    qDebug() << "end change access on init...";
}

int AccessControlManager::accessMode(const QString &mps)
{
    if (mps.isEmpty())
        return POLICY_DISABLE;
    char *path = mps.toLocal8Bit().data();
    if (access(path, W_OK) == 0)
        return POLICY_RW;
    if (access(path, R_OK) == 0)
        return POLICY_RONLY;
    return POLICY_DISABLE;
}

void AccessControlManager::loadPolicy()
{
    QFile config(m_configPath);
    if (!config.open(QIODevice::ReadOnly))
        return;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    if (doc.isArray()) {
        m_globalPolicies.clear();
        QJsonArray arr = doc.array();
        foreach (auto item, arr) {
            if (!item.isObject())
                continue;

            QJsonObject obj = item.toObject();
            int global = obj.contains(KEY_GLOBAL) ? obj.value(KEY_GLOBAL).toInt() : -1;
            if (global != 1) {
                // load black/white policy
                ;
            } else {
                // load default/general policy
                int type = obj.contains(KEY_TYPE) ? obj.value(KEY_TYPE).toInt() : TYPE_INVALID;
                int policy = obj.contains(KEY_POLICY) ? obj.value(KEY_POLICY).toInt() : POLICY_RW;
                QString source = obj.contains(KEY_INVOKER) ? obj.value(KEY_INVOKER).toString() : "";
                if (!m_globalPolicies.contains(type) || 1) // 1: 新读取到的数据源优先级大于之前读取到的，则刷新原有的
                    m_globalPolicies.insert(type, QPair<QString, int>(source, policy));
            }
        }
    }

    qDebug() << "loaded policy: " << m_globalPolicies;
}

void AccessControlManager::decodeConfig()
{

}

void AccessControlManager::encodeConfig()
{

}

bool AccessControlManager::isValidVaultPolicy(const QVariantMap &policy)
{
    if(policy.value(POLICYTYPE).toInt() < 0 || policy.value(VAULTHIDESTATE).toInt() < 0)
            return false;
    return true;
}

void AccessControlManager::saveVaultPolicy(const QVariantMap &policy)
{
    // 1. if file does not exist then create it
    QFile config(m_vaultConfigPath);
    if (!config.open(QIODevice::ReadWrite)) {
        qDebug() << "config open failed";
        config.close();
        return;
    }
    config.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    QJsonArray newArr;

    QJsonObject obj;
    obj.insert(POLICYTYPE, policy.value(POLICYTYPE).toInt());
    obj.insert(VAULTHIDESTATE, policy.value(VAULTHIDESTATE).toInt());
    obj.insert(POLICYSTATE, policy.value(POLICYSTATE).toInt());
    newArr.append(obj);
    qDebug() << "append new policy";
    doc.setArray(newArr);
    config.open(QIODevice::Truncate | QIODevice::ReadWrite); // overwrite the config file
    config.write(doc.toJson());
    config.close();
}

void AccessControlManager::loadVaultPolicy()
{
    QFile config(m_vaultConfigPath);
    if (!config.open(QIODevice::ReadOnly))
        return;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    if (doc.isArray()) {
        m_VaultHidePolicies.clear();
        QJsonArray arr = doc.array();
        foreach (auto item, arr) {
            if (!item.isObject())
                continue;

            QJsonObject obj = item.toObject();

            // load default/general policy
            int policytype = obj.contains(POLICYTYPE) ? obj.value(POLICYTYPE).toInt() : -1;
            int vaulthidestate = obj.contains(VAULTHIDESTATE) ? obj.value(VAULTHIDESTATE).toInt() : -1;
            int policystate = obj.contains(POLICYSTATE) ? obj.value(POLICYSTATE).toInt() : -1;
            m_VaultHidePolicies.insert(POLICYTYPE, policytype);
            m_VaultHidePolicies.insert(VAULTHIDESTATE, vaulthidestate);
            m_VaultHidePolicies.insert(POLICYSTATE, policystate);
        }
    }

    qDebug() << "loaded policy: " << m_VaultHidePolicies;
}

void AccessControlManager::onFileCreated(const QString &path, const QString &name)
{
    Q_UNUSED(path)
    Q_UNUSED(name)

    QDir homeDir("/home");
    QStringList dirNames = homeDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    dirNames.append("root"); // root 用户不在/home下，但设备会挂载到/media/root
    for (const QString &dirName : dirNames) {
        // /media/[UserName] 为默认挂载的基路径，预先从创建此目录，目的是为了确保该路径其他用户能够访问
        QString mountBaseName = QString("/media/%1").arg(dirName);
        QDir mountDir(mountBaseName);
        if (!mountDir.exists()) {
            qInfo() << mountBaseName << "not exists";
            if (QDir().mkpath(mountBaseName)) {
                qInfo() << "create" << mountBaseName << "success";
                struct stat fileStat;
                QByteArray nameBytes(mountBaseName.toUtf8());
                stat(nameBytes.data(), &fileStat);
                chmod(nameBytes.data(), (fileStat.st_mode | S_IRUSR | S_IRGRP | S_IROTH));
            }
        }
        // ACL
        QString aclCmd = QString("setfacl -m o:rx %1").arg(mountBaseName);
        QProcess::execute(aclCmd);
        qInfo() << "acl:" << aclCmd;
    }

}
