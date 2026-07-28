// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/repairdialog.h"
#include "usbrepair_interface.h"

#include <DApplication>
#include <QDBusConnection>
#include <QProcess>
#include <QFile>
#include <QCommandLineParser>
#include <QTimer>

DWIDGET_USE_NAMESPACE

using UsbRepairIface = OrgDeepinFilemanagerUsbRepairInterface;

static constexpr char kServiceName[] = "org.deepin.Filemanager.UsbRepair";
static constexpr char kServicePath[] = "/org/deepin/Filemanager/UsbRepair";

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setApplicationName("dde-usb-repair-dialog");
    app.setQuitOnLastWindowClosed(true);

    QCommandLineParser parser;
    parser.setApplicationDescription("USB Repair Dialog");
    parser.addHelpOption();
    QCommandLineOption devicePathOpt({ "d", "device-path" },
        "Block device path (e.g. /dev/sda1)", "path");
    QCommandLineOption deviceNameOpt({ "n", "device-name" },
        "Display name of the device", "name");
    QCommandLineOption fsTypeOpt({ "f", "fs-type" },
        "Filesystem type (e.g. vfat)", "type");
    QCommandLineOption deviceSizeOpt({ "s", "device-size" },
        "Formatted device size string", "size");
    parser.addOptions({ devicePathOpt, deviceNameOpt, fsTypeOpt, deviceSizeOpt });
    parser.process(app);

    QString devicePath = parser.value(devicePathOpt);
    QString deviceName = parser.value(deviceNameOpt);
    QString fsType = parser.value(fsTypeOpt);
    QString deviceSize = parser.value(deviceSizeOpt);

    if (devicePath.isEmpty()) {
        qWarning("No device path provided");
        return 1;
    }

    // Connect to USB repair service on system bus
    UsbRepairIface repairIface(kServiceName, kServicePath, QDBusConnection::systemBus());
    const QString ifaceError = repairIface.lastError().message();

    RepairDialog dialog;
    dialog.setDeviceInfo(deviceName, deviceSize, fsType.toUpper());
    dialog.setDevicePath(devicePath, QString());

    if (!repairIface.isValid()) {
        // Service unavailable: surface the error instead of leaving the dialog
        // stuck in a repairing state after the user clicks "Start Repair".
        qWarning() << "Cannot connect to UsbRepair service:" << ifaceError;
        dialog.setErrorCode(QObject::tr("Cannot connect to repair service"));
        dialog.setState(RepairDialog::kFailed);
    } else {
        dialog.setState(RepairDialog::kConfirm);
    }

    // Track whether repair has been started to avoid double-start
    bool repairStarted = false;

    // Handle confirmation dialog buttons
    QObject::connect(&dialog, &RepairDialog::buttonClicked, &app,
        [&](int index, const QString &) {

            switch (dialog.getState()) {
            case RepairDialog::kConfirm:
                if (index == 1 && !repairStarted) {
                    repairStarted = true;
                    QTimer::singleShot(0, &app, [&]() {
                        dialog.setState(RepairDialog::kRepairing);

                        QDBusPendingCall call = repairIface.asyncCall("Repair", devicePath);
                        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, &app);
                        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, &app,
                            [](QDBusPendingCallWatcher *w) {
                                QDBusPendingReply<bool, QString> reply = *w;
                                if (reply.isError()) {
                                    qWarning() << "Repair async call failed:" << reply.error().message();
                                }
                                w->deleteLater();
                            });
                    });
                } else {
                    app.quit();
                }
                break;
            case RepairDialog::kSuccess:
                if (index == 1) {
                    // "Open Device" button
                    QString mp = dialog.mountPoint();
                    if (!mp.isEmpty())
                        QProcess::startDetached("dde-file-manager", { mp });
                }
                app.quit();
                break;
            case RepairDialog::kFailed:
                app.quit();
                break;
            default:
                break;
            }
        });

    // Listen for repair progress signals
    QObject::connect(&repairIface, &UsbRepairIface::RepairProgress,
        &dialog, [&dialog, devicePath](const QString &path, int percent, const QString &) {
            if (path == devicePath)
                dialog.setProgress(percent);
        });

    // Listen for repair finished signals
    QObject::connect(&repairIface, &UsbRepairIface::RepairFinished,
        &dialog, [&dialog, &app, devicePath](const QString &path, bool success, const QString &summary) {
            if (path != devicePath)
                return;

            QTimer::singleShot(0, &app, [&dialog, &app, devicePath, success, summary]() {
                if (success) {
                    // Mount asynchronously to avoid blocking the event loop,
                    // which causes the window to appear greyed out under Wayland.
                    QProcess *mountProc = new QProcess(&app);
                    QObject::connect(mountProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        &app, [&dialog, devicePath, mountProc](int, QProcess::ExitStatus) {
                            QString mountPoint;
                            QFile mounts("/proc/mounts");
                            if (mounts.open(QIODevice::ReadOnly)) {
                                QByteArray data = mounts.readAll();
                                mounts.close();
                                for (const QByteArray &line : data.split('\n')) {
                                    QList<QByteArray> parts = line.split(' ');
                                    if (parts.size() >= 2 && parts[0] == devicePath.toUtf8()) {
                                        mountPoint = QString::fromUtf8(parts[1]);
                                        break;
                                    }
                                }
                            }

                            if (mountPoint.isEmpty()) {
                                dialog.setErrorCode(QObject::tr("Mount failed after repair"));
                                dialog.setState(RepairDialog::kFailed);
                            } else {
                                dialog.setDevicePath(devicePath, mountPoint);
                                dialog.setState(RepairDialog::kSuccess);
                            }
                            mountProc->deleteLater();
                        });
                    mountProc->start("udisksctl", { "mount", "-b", devicePath });
                } else {
                    dialog.setErrorCode(summary);
                    dialog.setState(RepairDialog::kFailed);
                }
            });
        });

    // Use show() + app.exec() instead of dialog.exec().
    // DDialog::exec() uses its own QEventLoop that returns on hideEvent,
    // and clearButtons()/clearContents() during state transitions can
    // trigger a hide, causing exec() to exit prematurely.
    dialog.show();
    return app.exec();
}
