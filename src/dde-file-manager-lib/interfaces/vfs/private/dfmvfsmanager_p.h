// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
