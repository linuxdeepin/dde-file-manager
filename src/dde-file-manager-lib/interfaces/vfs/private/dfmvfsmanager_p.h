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

#ifndef DFMVFSMANAGER_P_H
#define DFMVFSMANAGER_P_H

#include "dfmvfsmanager.h"
#include "dfmvfsdevice.h"
#include "private/dfmgiowrapper_p.h"

DFM_BEGIN_NAMESPACE

class DFMVfsManagerPrivate
{
    Q_DECLARE_PUBLIC(DFMVfsManager)

public:
    explicit DFMVfsManagerPrivate(DFMVfsManager *qq);
    ~DFMVfsManagerPrivate();

    QScopedPointer<GVolumeMonitor, ScopedPointerGObjectUnrefDeleter> m_GVolumeMonitor;
    DFMVfsAbstractEventHandler *m_handler = nullptr;
    QPointer<QThread> m_threadOfEventHandler;

    static void GVolumeMonitorMountAddedCb(GVolumeMonitor *, GMount *mount, DFMVfsManager* managerPointer);
    static void GVolumeMonitorMountRemovedCb(GVolumeMonitor *, GMount *mount, DFMVfsManager* managerPointer);
    static void GVolumeMonitorMountChangedCb(GVolumeMonitor *, GMount *mount, DFMVfsManager* managerPointer);


    DFMVfsManager *q_ptr = nullptr;

private:
    void initConnect();
};

DFM_END_NAMESPACE

#endif // DFMVFSMANAGER_P_H
