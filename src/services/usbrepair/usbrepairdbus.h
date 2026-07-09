// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USBREPAIRDBUS_H
#define USBREPAIRDBUS_H

#include "service_usbrepair_global.h"

#include <QObject>
#include <QDBusContext>

SERVICEUSBREPAIR_BEGIN_NAMESPACE
class UsbRepairDBusPrivate;
SERVICEUSBREPAIR_END_NAMESPACE

class UsbRepairDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.UsbRepair")

public:
    explicit UsbRepairDBus(const char *name, QObject *parent = nullptr);
    ~UsbRepairDBus();

    void cleanup();

public Q_SLOTS:
    bool Repair(const QString &devicePath, QString &errorMessage);
    bool CancelRepair(const QString &devicePath);

Q_SIGNALS:
    void FsErrorDetected(const QString &devicePath,
                         const QString &deviceName,
                         const QString &fsType,
                         const QString &errorType,
                         bool canRepair,
                         const QString &message);
    void RepairProgress(const QString &devicePath,
                        int percent,
                        const QString &logLine);
    void RepairFinished(const QString &devicePath,
                        bool success,
                        const QString &summary);
    void FsErrorCleared(const QString &devicePath);

private:
    QScopedPointer<SERVICEUSBREPAIR_NAMESPACE::UsbRepairDBusPrivate> d;
};

#endif   // USBREPAIRDBUS_H
