// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>

#include "dfmglobal.h"

class BluetoothModel;
class BluetoothDevice;
class BluetoothAdapter;
class BluetoothManagerPrivate;

/**
 * @brief This is singleton class.
 * you should `#include "app/define.h"` and `"bluetooth/bluetoothmanager.h"`,
 * then use the macro `bluetoothManager` call method
 */
class BluetoothManager : public QObject
{
    Q_OBJECT

public:
    static BluetoothManager *instance();

    /**
     * @brief 返回数据模型
     * @return
     */
    BluetoothModel *model();

    bool hasAdapter();

    static bool bluetoothSendEnable();


public Q_SLOTS:
    /**
     * @brief 异步接口：调用刷新蓝牙模块，初始化数据
     */
    void refresh();

    /**
     * @brief 打开控制中心的‘蓝牙’界面
     */
    void showBluetoothSettings();

    /**
     * @brief sendFiles     向设备发送文件
     * @param id            蓝牙设备 ID
     * @param filePath      文件路径列表
     * @param senderToken   发起该次传输请求的对话框唯一标识符
     */
    void sendFiles(const QString &id, const QStringList &filePath, const QString &senderToken);

    /**
     * @brief cancleTransfer 取消某个传输会话
     * @param sessionPath
     * @return
     */
    bool cancelTransfer(const QString &sessionPath);

    bool canSendBluetoothRequest();

Q_SIGNALS:

    /**
     * @brief transferProcessUpdated    当前传输会话进度更新
     * @param sessionPath   用来标识当前传输进程的会话id
     * @param total         当前传输列表的总bytes
     * @param transferred   当前已传输的 bytes
     * @param currFileIndex 当前传输文件的序号
     */
    void transferProgressUpdated(const QString &sessionPath, qulonglong total, qulonglong transferred, int currFileIndex);

    /**
     * @brief transferCancledByRemote
     * @param sessionPath
     */
    void transferCancledByRemote(const QString &sessionPath);

    void fileTransferFinished(const QString &sessionPath, const QString &filePath);
    void transferFailed(const QString &sessionPath, const QString &filePath, const QString &errMsg);
    void transferEstablishFinish(const QString &sessionPath, const QString &errMsg, const QString &senderToken);

private:
    explicit BluetoothManager(QObject *parent = nullptr);
    BluetoothManager(const BluetoothManager &) = delete;
    BluetoothManager &operator = (const BluetoothManager &) = delete;

    QSharedPointer<BluetoothManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(BluetoothManager)
};

#endif // BLUETOOTHMANAGER_H
