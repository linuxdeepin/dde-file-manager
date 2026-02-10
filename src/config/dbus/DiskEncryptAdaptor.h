// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKENCRYPTADAPTOR_H
#define DISKENCRYPTADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.Filemanager.DiskEncrypt
 */
class DiskEncryptAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.DiskEncrypt")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.DiskEncrypt\">\n"
"    <signal name=\"PrepareEncryptDiskResult\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"device\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"devName\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"jobID\"/>\n"
"      <arg direction=\"out\" type=\"i\" name=\"errCode\"/>\n"
"    </signal>\n"
"    <signal name=\"EncryptDiskResult\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"device\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"devName\"/>\n"
"      <arg direction=\"out\" type=\"i\" name=\"errCode\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"msg\"/>\n"
"    </signal>\n"
"    <signal name=\"DecryptDiskResult\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"device\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"devName\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"jobID\"/>\n"
"      <arg direction=\"out\" type=\"i\" name=\"errCode\"/>\n"
"    </signal>\n"
"    <signal name=\"ChangePassphressResult\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"device\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"devName\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"jobID\"/>\n"
"      <arg direction=\"out\" type=\"i\" name=\"errCode\"/>\n"
"    </signal>\n"
"    <signal name=\"EncryptProgress\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"device\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"devName\"/>\n"
"      <arg direction=\"out\" type=\"d\" name=\"progress\"/>\n"
"    </signal>\n"
"    <signal name=\"DecryptProgress\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"device\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"devName\"/>\n"
"      <arg direction=\"out\" type=\"d\" name=\"progress\"/>\n"
"    </signal>\n"
"    <signal name=\"RequestEncryptParams\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"encConfig\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <method name=\"PrepareEncryptDisk\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"params\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"DecryptDisk\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"params\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"ChangeEncryptPassphress\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"params\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"IgnoreParamRequest\"/>\n"
"    <method name=\"ResumeEncryption\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"device\"/>\n"
"    </method>\n"
"    <method name=\"SetEncryptParams\">\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"params\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"QueryTPMToken\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"device\"/>\n"
"    </method>\n"
"    <method name=\"EncryptStatus\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"device\"/>\n"
"    </method>\n"
"    <method name=\"HasPendingTask\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"IsWorkerRunning\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"UnfinishedDecryptJob\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    DiskEncryptAdaptor(QObject *parent);
    virtual ~DiskEncryptAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QString ChangeEncryptPassphress(const QVariantMap &params);
    QString DecryptDisk(const QVariantMap &params);
    int EncryptStatus(const QString &device);
    bool HasPendingTask();
    void IgnoreParamRequest();
    bool IsWorkerRunning();
    QString PrepareEncryptDisk(const QVariantMap &params);
    QString QueryTPMToken(const QString &device);
    void ResumeEncryption(const QString &device);
    void SetEncryptParams(const QVariantMap &params);
    QString UnfinishedDecryptJob();
Q_SIGNALS: // SIGNALS
    void ChangePassphressResult(const QString &device, const QString &devName, const QString &jobID, int errCode);
    void DecryptDiskResult(const QString &device, const QString &devName, const QString &jobID, int errCode);
    void DecryptProgress(const QString &device, const QString &devName, double progress);
    void EncryptDiskResult(const QString &device, const QString &devName, int errCode, const QString &msg);
    void EncryptProgress(const QString &device, const QString &devName, double progress);
    void PrepareEncryptDiskResult(const QString &device, const QString &devName, const QString &jobID, int errCode);
    void RequestEncryptParams(const QVariantMap &encConfig);
};

#endif
