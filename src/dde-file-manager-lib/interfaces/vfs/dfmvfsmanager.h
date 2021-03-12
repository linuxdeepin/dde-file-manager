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

#ifndef DFMVFSMANAGER_H
#define DFMVFSMANAGER_H

#include "dfmvfsabstracteventhandler.h"
#include <dfmglobal.h>
#include <QObject>

DFM_BEGIN_NAMESPACE

class DFMVfsManagerPrivate;
class DFMVfsManager : public QObject {
    Q_OBJECT
public:
    explicit DFMVfsManager(QObject *parent = nullptr);
    ~DFMVfsManager();

    const QList<QUrl> getVfsList(); // 已经挂载的所有网络设备列表

    bool attach(const QUrl& url);

    DFMVfsAbstractEventHandler *eventHandler() const;
    void setEventHandler(DFMVfsAbstractEventHandler *handler, QThread *threadOfHandle = nullptr);

signals:
    void vfsAttached(const QUrl& url); // 参数为挂载点 url ，可以直接通过挂载点 url ，当存在 shadow 挂载点时一个设备可能有多个挂载信号
    void vfsDetached(const QUrl& url); // 来创建一个 DFMVfsDevice ，存在 shadow 挂载点时也可能有多个卸载信号
    void vfsDeviceListInfoChanged(); // 有 vfs 的设备信息改变的通知，改变也包括类如多了个 shadow 挂载点之类的属性等

private:
    QScopedPointer<DFMVfsManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMVfsManager)
};

DFM_END_NAMESPACE

#endif // DFMVFSMANAGER_H
