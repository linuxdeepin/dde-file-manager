#include "defenderinterface.h"

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QApplication>

#define DBUS_SERVICE_NAME "com.deepin.defender.daemonservice"
#define DBUS_SERVICE_PATH "/com/deepin/defender/daemonservice"
#define DBUS_INTERFACE_NAME "com.deepin.defender.daemonservice"

DefenderInterface::DefenderInterface(QObject *parent)
    :QObject(parent)
{
    interface = new QDBusInterface(DBUS_SERVICE_NAME,
                         DBUS_SERVICE_PATH,
                         DBUS_INTERFACE_NAME,
                         QDBusConnection::sessionBus());

    QDBusConnection::sessionBus().connect(
        DBUS_SERVICE_NAME,
        DBUS_SERVICE_PATH,
        DBUS_INTERFACE_NAME,
        "ScanningUsbPathsChanged",
        this,
        SLOT(scanningUsbPathsChanged(QStringList)));

    QStringList list = interface->property("ScanningUsbPaths").toStringList();
    foreach (const QString &p, list)
        scanningPaths << QUrl::fromLocalFile(p);
}

void DefenderInterface::scanningUsbPathsChanged(QStringList list)
{
    qInfo() << "reveive signal: scanningUsbPathsChanged, " << list;
    scanningPaths.clear();
    foreach (const QString &p, list)
        scanningPaths << QUrl::fromLocalFile(p);
}

DefenderInterface::~DefenderInterface()
{
    if (interface) {
        delete interface;
        interface = nullptr;
    }
}

// 从当前扫描路径选取url的子目录，若url本身也被扫描，也会在返回列表中
QList<QUrl> DefenderInterface::getScanningPaths(const QUrl &url)
{
    QList<QUrl> list;
    foreach (const QUrl &p, scanningPaths) {
        if (url.isParentOf(p) || url == p)
            list << p;
    }
    return list;
}

// 停止扫描url及url的子目录，返回false为停止失败
bool DefenderInterface::stopScanning(const QUrl &url)
{
    qInfo() << "stopScanning:" << url;
    QList<QUrl> paths = getScanningPaths(url);
    qInfo() << "scanning child:" << paths;

    if (paths.empty())
        return true;

    foreach (const QUrl &path, paths) {
        qInfo() << "send RequestStopUsbScannig:" << path;
        interface->asyncCall("RequestStopUsbScannig", path.toLocalFile());
    }

    // 等待扫描目录改变信号，直到超时
    QTime t;
    t.start();
    while (t.elapsed() < 200) {
        qApp->processEvents();
        if (!isScanning(url))
            return true;
        QThread::msleep(10);
    }
    return false;
}

// 检查当前扫描路径中是否包含url或url的子目录
bool DefenderInterface::isScanning(const QUrl &url)
{
    QList<QUrl> paths = getScanningPaths(url);
    return !paths.empty();
}
