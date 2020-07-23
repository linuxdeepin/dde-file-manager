/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
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
     * @brief 向设备发送文件
     * @param device 设备对象
     * @param filePath 文件路径
     * @return 文件可以发送返回 true，但并不意味着发送成功
     */
    bool sendFile(const BluetoothDevice &device, const QString &filePath);

    /**
     * @brief 向设备发送文件
     * @param id 设备对象中的id
     * @param filePath 文件路径
     * @return 文件可以发送返回 true，，但并不意味着发送成功
     */
    bool sendFile(const QString &id, const QString &filePath);

private:
    explicit BluetoothManager(QObject *parent = nullptr);
    BluetoothManager(const BluetoothManager &) = delete;
    BluetoothManager &operator = (const BluetoothManager &) = delete;

    QSharedPointer<BluetoothManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(BluetoothManager)
};

#endif // BLUETOOTHMANAGER_H
