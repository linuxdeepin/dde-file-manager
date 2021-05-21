#include <gtest/gtest.h>
#include "stub.h"
#include "stubext.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QStandardPaths>
#include <QDir>
#include <time.h>
#include <QEventLoop>
#include <QTimer>
#include <QFileInfo>
#include <QFile>

#include <ddiskmanager.h>
#include <ddiskdevice.h>
#include <dblockdevice.h>

#define private public
#define protected public
#include "accesscontrol/accesscontrolmanager.h"

namespace {
class TestAccessControlManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestAccessControlManager";
        mng = new AccessControlManager;
    }

    void TearDown() override
    {
        std::cout << "end TestAccessControlManager";
        delete mng;
        mng = nullptr;
    }

public:
    AccessControlManager *mng = nullptr;
};
} // namespace

TEST_F(TestAccessControlManager, testCheckAuthentication1)
{
    bool(*stu_isConnected)() = []()->bool{
        return false;
    };
    Stub stu1;
    stu1.set(ADDR(QDBusConnection, isConnected), stu_isConnected);

    EXPECT_FALSE(mng->checkAuthentication());

}

TEST_F(TestAccessControlManager, testCheckAuthentication2)
{
    // 打桩满足特殊条件
    uint(*stu_value)() = []()->uint{
            return 123123;
    };
    Stub stu1;
    stu1.set(ADDR(QDBusReply<uint>, value), stu_value);

    QDBusConnectionInterface *(*stu_interface)() = []()->QDBusConnectionInterface *{
        QObject *pobj = new QObject();
        return static_cast<QDBusConnectionInterface *>(pobj);
    };
    Stub stu3;
    stu3.set(ADDR(QDBusConnection, interface), stu_interface);

    QDBusMessage(*stu_message)() = []()->QDBusMessage{
        return QDBusMessage();
    };
    Stub stu4;
    stu4.set(ADDR(QDBusContext, message), stu_message);

    QDBusReply<uint>(*stu_servicePid)(const QString &) = [](const QString &)->QDBusReply<uint>{
        return QDBusReply<uint>();
    };
    Stub stu5;
    stu5.set(ADDR(QDBusConnectionInterface, servicePid), stu_servicePid);

    EXPECT_FALSE(mng->checkAuthentication());

}

TEST_F(TestAccessControlManager, testQueryAccessPolicy)
{
    // 组织数据
    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(1, QPair<QString, int>("/usr/bin/python3.7", 0));
    mng->m_globalPolicies.insert(2, QPair<QString, int>("/usr/bin/python3.7", 0));

    QVariantList list = mng->QueryAccessPolicy();

    int nCount = list.count();
    EXPECT_EQ(nCount, 2);
}

TEST_F(TestAccessControlManager, testSetAccessPolicy1)
{
    // 打桩满足特殊条件
    uint(*stu_value)() = []()->uint{
            return 123123;
    };
    Stub stu1;
    stu1.set(ADDR(QDBusReply<uint>, value), stu_value);

    QDBusConnection(*stu_connect)() = []()->QDBusConnection{
        return QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    };
    Stub stu2;
    stu2.set(ADDR(QDBusContext, connection), stu_connect);

    QDBusConnectionInterface *(*stu_interface)() = []()->QDBusConnectionInterface *{
        QObject *pobj = new QObject();
        return static_cast<QDBusConnectionInterface *>(pobj);
    };
    Stub stu3;
    stu3.set(ADDR(QDBusConnection, interface), stu_interface);

    QDBusMessage(*stu_message)() = []()->QDBusMessage{
        return QDBusMessage();
    };
    Stub stu4;
    stu4.set(ADDR(QDBusContext, message), stu_message);

    QDBusReply<uint>(*stu_servicePid)(const QString &) = [](const QString &)->QDBusReply<uint>{
        return QDBusReply<uint>();
    };
    Stub stu5;
    stu5.set(ADDR(QDBusConnectionInterface, servicePid), stu_servicePid);

    QVariantMap policy;
    policy.insert("invoker", "test");
    policy.insert("policy", 1);
    policy.insert("type", 2);
    EXPECT_NO_FATAL_FAILURE(mng->SetAccessPolicy(policy));
}

TEST_F(TestAccessControlManager, testSetAccessPolicy2)
{
    // 打桩满足特殊条件
    uint(*stu_value)() = []()->uint{
            return 123123;
    };
    Stub stu1;
    stu1.set(ADDR(QDBusReply<uint>, value), stu_value);

    QDBusConnection(*stu_connect)() = []()->QDBusConnection{
        return QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    };
    Stub stu2;
    stu2.set(ADDR(QDBusContext, connection), stu_connect);

    QDBusConnectionInterface *(*stu_interface)() = []()->QDBusConnectionInterface *{
        QObject *pobj = new QObject();
        return static_cast<QDBusConnectionInterface *>(pobj);
    };
    Stub stu3;
    stu3.set(ADDR(QDBusConnection, interface), stu_interface);

    QDBusMessage(*stu_message)() = []()->QDBusMessage{
        return QDBusMessage();
    };
    Stub stu4;
    stu4.set(ADDR(QDBusContext, message), stu_message);

    QDBusReply<uint>(*stu_servicePid)(const QString &) = [](const QString &)->QDBusReply<uint>{
        return QDBusReply<uint>();
    };
    Stub stu5;
    stu5.set(ADDR(QDBusConnectionInterface, servicePid), stu_servicePid);

    bool(*stu_isValidInvoker)(uint, QString &) = [](uint, QString &)->bool{
        return true;
    };
    Stub stu6;
    stu6.set(ADDR(AccessControlManager, isValidInvoker), stu_isValidInvoker);

    QVariantMap policy;
    policy.insert("invoker", "test");
    policy.insert("policy", 1);
    policy.insert("type", 2);
    EXPECT_NO_FATAL_FAILURE(mng->SetAccessPolicy(policy));
}

TEST_F(TestAccessControlManager, testSetAccessPolicy3)
{
    // 打桩满足特殊条件
    uint(*stu_value)() = []()->uint{
            return 123123;
    };
    Stub stu1;
    stu1.set(ADDR(QDBusReply<uint>, value), stu_value);

    QDBusConnection(*stu_connect)() = []()->QDBusConnection{
        return QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    };
    Stub stu2;
    stu2.set(ADDR(QDBusContext, connection), stu_connect);

    QDBusConnectionInterface *(*stu_interface)() = []()->QDBusConnectionInterface *{
        QObject *pobj = new QObject();
        return static_cast<QDBusConnectionInterface *>(pobj);
    };
    Stub stu3;
    stu3.set(ADDR(QDBusConnection, interface), stu_interface);

    QDBusMessage(*stu_message)() = []()->QDBusMessage{
        return QDBusMessage();
    };
    Stub stu4;
    stu4.set(ADDR(QDBusContext, message), stu_message);

    QDBusReply<uint>(*stu_servicePid)(const QString &) = [](const QString &)->QDBusReply<uint>{
        return QDBusReply<uint>();
    };
    Stub stu5;
    stu5.set(ADDR(QDBusConnectionInterface, servicePid), stu_servicePid);

    bool(*stu_isValidInvoker)(uint, QString &) = [](uint, QString &)->bool{
        return true;
    };
    Stub stu6;
    stu6.set(ADDR(AccessControlManager, isValidInvoker), stu_isValidInvoker);

    bool(*stu_isValidPolicy)(const QVariantMap &, const QString &) = [](const QVariantMap &, const QString &)->bool{
        return true;
    };
    Stub stu7;
    stu7.set(ADDR(AccessControlManager, isValidPolicy), stu_isValidPolicy);

    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(1, QPair<QString, int>("/usr/bin/python3.7", 0));
    QVariantMap policy;
    policy.insert("invoker", "test");
    policy.insert("policy", 1);
    policy.insert("type", 2);
    EXPECT_NO_FATAL_FAILURE(mng->SetAccessPolicy(policy));
}

TEST_F(TestAccessControlManager, testOnFileCreated)
{
    // 打桩满足特殊条件
    bool(*stu_exists)() = []()->bool{return false;};
    Stub stu1;
    bool(QDir::*m_exists)()const = &QDir::exists;
    stu1.set(m_exists, stu_exists);

    EXPECT_NO_FATAL_FAILURE(mng->onFileCreated("", ""));
}

TEST_F(TestAccessControlManager, testChmodMountpoints1)
{
    // 组织数据
    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(1, QPair<QString, int>("/usr/bin/python3.7", 0));
    mng->m_globalPolicies.insert(2, QPair<QString, int>("/usr/bin/python3.7", 0));
    const QString blockDevicePath = "/usr/bin/python3.7";
    const QByteArray mountPoint = "/media/root";

    EXPECT_NO_FATAL_FAILURE(mng->chmodMountpoints(blockDevicePath, mountPoint));
}

TEST_F(TestAccessControlManager, testChmodMountpoints2)
{
    // 组织数据
    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(1, QPair<QString, int>("/usr/bin/python3.7", 1));
    mng->m_globalPolicies.insert(2, QPair<QString, int>("/usr/bin/python3.7", 1));
    const QString blockDevicePath = "/usr/bin/python3.7";
    QByteArray mountPoint = "/media/root";
    // 普通用户时，设置一个可写的挂载点
    if (getuid() != 0) {
        QString strHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        long t = time(nullptr);
        QString dirName = QString("mountPoint_W_%1").arg(t);
        QString strDirPath = strHome + QDir::separator() + dirName;
        QDir dir(strDirPath);
        if (dir.mkdir(strDirPath)) {
            if (QFile(strDirPath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
                mountPoint = strDirPath.toUtf8();
        }
    }

    EXPECT_NO_FATAL_FAILURE(mng->chmodMountpoints(blockDevicePath, mountPoint));
}

TEST_F(TestAccessControlManager, testChmodMountpoints3)
{
    // 组织数据
    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(1, QPair<QString, int>("/usr/bin/python3.7", 2));
    mng->m_globalPolicies.insert(2, QPair<QString, int>("/usr/bin/python3.7", 2));
    const QString blockDevicePath = "/usr/bin/python3.7";
    QByteArray mountPoint = "/media/root";

    QString strDirPath;
    bool bCommon = false;
    // 普通用户时，设置一个可写的挂载点
    if (getuid() != 0) {
        bCommon = true;
        QString strHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        long t = time(nullptr);
        QString dirName = QString("mountPoint_W_%1").arg(t);
        strDirPath = strHome + QDir::separator() + dirName;
        QDir dir(strDirPath);
        if (dir.mkdir(strDirPath)) {
            if (QFile(strDirPath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
                mountPoint = strDirPath.toUtf8();
        }
    }

    EXPECT_NO_FATAL_FAILURE(mng->chmodMountpoints(blockDevicePath, mountPoint));

    // 删除挂载点
    if (bCommon)
        QDir(strDirPath).rmdir(strDirPath);
}

TEST_F(TestAccessControlManager, testDisconnOpticalDev1)
{
    const QString &drivePath = "/org/freedesktop/UDisk123";
    EXPECT_NO_FATAL_FAILURE(mng->disconnOpticalDev(drivePath));
}

TEST_F(TestAccessControlManager, testDisconnOpticalDev2)
{
    // 此接口用户卸载设备，目前无法模拟挂载设备，故打桩实现
    bool(*stu_canPowerOff)() = []()->bool{
        return true;
    };
    Stub stu1;
    stu1.set(ADDR(DDiskDevice, canPowerOff), stu_canPowerOff);

    QString(*stu_connectionBus)() = []()->QString{
        return "usb";
    };
    Stub stu2;
    stu2.set(ADDR(DDiskDevice, connectionBus), stu_connectionBus);

    const QString &drivePath = "/org/freedesktop/UDisk123";
    EXPECT_NO_FATAL_FAILURE(mng->disconnOpticalDev(drivePath));
}

TEST_F(TestAccessControlManager, testDisconnOpticalDev3)
{
    // 此接口用户卸载设备，目前无法模拟挂载设置，故打桩实现
    bool(*stu_canPowerOff)() = []()->bool{
        return true;
    };
    Stub stu1;
    stu1.set(ADDR(DDiskDevice, canPowerOff), stu_canPowerOff);

    QString(*stu_connectionBus)() = []()->QString{
        return "usb";
    };
    Stub stu2;
    stu2.set(ADDR(DDiskDevice, connectionBus), stu_connectionBus);

    QStringList(*stu_mediaCompatibility)() = []()->QStringList{
            return QStringList({"optical"});
    };
    Stub stu3;
    stu3.set(ADDR(DDiskDevice, mediaCompatibility), stu_mediaCompatibility);

    const QString &drivePath = "/org/freedesktop/UDisk123";
    EXPECT_NO_FATAL_FAILURE(mng->disconnOpticalDev(drivePath));
}

TEST_F(TestAccessControlManager, testDisconnOpticalDev4)
{
    // 此接口用户卸载设备，目前无法模拟挂载设置，故打桩实现
    bool(*stu_canPowerOff)() = []()->bool{
        return true;
    };
    Stub stu1;
    stu1.set(ADDR(DDiskDevice, canPowerOff), stu_canPowerOff);

    QString(*stu_connectionBus)() = []()->QString{
        return "usb";
    };
    Stub stu2;
    stu2.set(ADDR(DDiskDevice, connectionBus), stu_connectionBus);

    QStringList(*stu_mediaCompatibility)() = []()->QStringList{
            return QStringList({"optical"});
    };
    Stub stu3;
    stu3.set(ADDR(DDiskDevice, mediaCompatibility), stu_mediaCompatibility);

    const QString &drivePath = "/org/freedesktop/UDisk123";
    mng->m_globalPolicies.insert(0x02, QPair<QString, int>("/usr/bin/python3.7", 1));
    EXPECT_NO_FATAL_FAILURE(mng->disconnOpticalDev(drivePath));
}

TEST_F(TestAccessControlManager, testDisconnOpticalDev5)
{
    // 此接口用户卸载设备，目前无法模拟挂载设置，故打桩实现
    bool(*stu_canPowerOff)() = []()->bool{
        return true;
    };
    Stub stu1;
    stu1.set(ADDR(DDiskDevice, canPowerOff), stu_canPowerOff);

    QString(*stu_connectionBus)() = []()->QString{
        return "usb";
    };
    Stub stu2;
    stu2.set(ADDR(DDiskDevice, connectionBus), stu_connectionBus);

    QStringList(*stu_mediaCompatibility)() = []()->QStringList{
            return QStringList({"optical"});
    };
    Stub stu3;
    stu3.set(ADDR(DDiskDevice, mediaCompatibility), stu_mediaCompatibility);

    const QString &drivePath = "/org/freedesktop/UDisk123";
    mng->m_globalPolicies.insert(0x02, QPair<QString, int>("/usr/bin/python3.7", 0));
    EXPECT_NO_FATAL_FAILURE(mng->disconnOpticalDev(drivePath));
}

TEST_F(TestAccessControlManager, testIsValidPolicy)
{
    const QString realInvoker = "/usr/bin/python3.7";
    QVariantMap policy;
    policy.insert("invoker", realInvoker);
    policy.insert("type", 7);
    policy.insert("policy", 2);

    EXPECT_TRUE(mng->isValidPolicy(policy, realInvoker));
}

TEST_F(TestAccessControlManager, testIsValidInvoker1)
{
    QString strInvoker = "/usr/bin/python3.7";
    EXPECT_FALSE(mng->isValidInvoker(123456, strInvoker));
}

TEST_F(TestAccessControlManager, testIsValidInvoker2)
{
    // 打桩满足条件
    bool(*stu_exists)() = []()->bool{
        return true;
    };
    Stub stu1;
    bool (QFileInfo::*m_exits)() const = &QFileInfo::exists;
    stu1.set(m_exits, stu_exists);

    QString strInvoker = "/usr/bin/python3.7";
    EXPECT_FALSE(mng->isValidInvoker(123456, strInvoker));
}

TEST_F(TestAccessControlManager, testChangeMountedPolicy_Block)
{
    QVariantMap policy;
    policy.insert("type", 0x01);
    policy.insert("policy", 0);
    policy.insert("device", "");
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedPolicy(policy));
}

TEST_F(TestAccessControlManager, testChangeMountedPolicy_Optical)
{
    QVariantMap policy;
    policy.insert("type", 0x02);
    policy.insert("policy", 0);
    policy.insert("device", "");
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedPolicy(policy));
}


TEST_F(TestAccessControlManager, testChangeMountedPolicy_Protocol)
{
    QVariantMap policy;
    policy.insert("type", 0x04);
    policy.insert("policy", 0);
    policy.insert("device", "");
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedPolicy(policy));
}

TEST_F(TestAccessControlManager, testChangeMountedBlock)
{
    // 打桩满足条件
    QStringList(*stu_blockDevices)(QVariantMap) = [](QVariantMap)->QStringList{
        return QStringList({"/org/freedesktop/UDisks123"});
    };
    Stub stu1;
    QStringList (*m_blockDevices)(QVariantMap) = &DDiskManager::blockDevices;
    stu1.set(m_blockDevices, stu_blockDevices);

    bool(*stu_hasFileSystem)() = []()->bool{
        return true;
    };
    Stub stu2;
    bool(DBlockDevice::*m_hasFileSystem)()const = &DBlockDevice::hasFileSystem;
    stu1.set(m_hasFileSystem, stu_hasFileSystem);

    QByteArrayList(*stu_mountPoints)() = []()->QByteArrayList{
        return QByteArrayList({"/123"});
    };
    Stub stu3;
    stu3.set(ADDR(DBlockDevice, mountPoints), stu_mountPoints);

    bool(*stu_removeble)() = []()->bool{
        return true;
    };
    Stub stu4;
    stu4.set(ADDR(DDiskDevice, removable), stu_removeble);

    bool(*stu_optical)() = []()->bool{
        return false;
    };
    Stub stu5;
    stu5.set(ADDR(DDiskDevice, optical), stu_optical);

    int(*stu_accessMode)(const QString&) = [](const QString&)->int{return 1;};
    Stub stu6;
    stu6.set(ADDR(AccessControlManager, accessMode), stu_accessMode);

    // 挂载点权限与策略一致
    int mode = 1;
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedBlock(mode));
}

TEST_F(TestAccessControlManager, testChangeMountedBlock2)
{
    // 打桩满足条件
    QStringList(*stu_blockDevices)(QVariantMap) = [](QVariantMap)->QStringList{
        return QStringList({"/org/freedesktop/UDisks123"});
    };
    Stub stu1;
    QStringList (*m_blockDevices)(QVariantMap) = &DDiskManager::blockDevices;
    stu1.set(m_blockDevices, stu_blockDevices);

    bool(*stu_hasFileSystem)() = []()->bool{
        return true;
    };
    Stub stu2;
    bool(DBlockDevice::*m_hasFileSystem)()const = &DBlockDevice::hasFileSystem;
    stu1.set(m_hasFileSystem, stu_hasFileSystem);

    QByteArrayList(*stu_mountPoints)() = []()->QByteArrayList{
        return QByteArrayList({"/123"});
    };
    Stub stu3;
    stu3.set(ADDR(DBlockDevice, mountPoints), stu_mountPoints);

    bool(*stu_removeble)() = []()->bool{
        return true;
    };
    Stub stu4;
    stu4.set(ADDR(DDiskDevice, removable), stu_removeble);

    bool(*stu_optical)() = []()->bool{
        return false;
    };
    Stub stu5;
    stu5.set(ADDR(DDiskDevice, optical), stu_optical);

    int(*stu_accessMode)(const QString&) = [](const QString&)->int{return 1;};
    Stub stu6;
    stu6.set(ADDR(AccessControlManager, accessMode), stu_accessMode);

    // 挂载点权限与策略不一致，需要卸载挂载点
    int mode = 0;
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedBlock(mode));
}

TEST_F(TestAccessControlManager, testChangeMountedBlock3)
{
    // 打桩满足条件
    QStringList(*stu_blockDevices)(QVariantMap) = [](QVariantMap)->QStringList{
        return QStringList({"/org/freedesktop/UDisks123"});
    };
    Stub stu1;
    QStringList (*m_blockDevices)(QVariantMap) = &DDiskManager::blockDevices;
    stu1.set(m_blockDevices, stu_blockDevices);

    bool(*stu_hasFileSystem)() = []()->bool{
        return true;
    };
    Stub stu2;
    bool(DBlockDevice::*m_hasFileSystem)()const = &DBlockDevice::hasFileSystem;
    stu1.set(m_hasFileSystem, stu_hasFileSystem);

    QByteArrayList(*stu_mountPoints)() = []()->QByteArrayList{
        return QByteArrayList({"/123"});
    };
    Stub stu3;
    stu3.set(ADDR(DBlockDevice, mountPoints), stu_mountPoints);

    bool(*stu_removeble)() = []()->bool{
        return true;
    };
    Stub stu4;
    stu4.set(ADDR(DDiskDevice, removable), stu_removeble);

    bool(*stu_optical)() = []()->bool{
        return false;
    };
    Stub stu5;
    stu5.set(ADDR(DDiskDevice, optical), stu_optical);

    int(*stu_accessMode)(const QString&) = [](const QString&)->int{return 0;};
    Stub stu6;
    stu6.set(ADDR(AccessControlManager, accessMode), stu_accessMode);

    // 挂载点权限与策略不一致，需要挂载挂载点
    int mode = 1;
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedBlock(mode));
}

//TEST_F(TestAccessControlManager, testChangeMountedOptical1)
//{

//}

TEST_F(TestAccessControlManager, testChangeMountedOptical2)
{
    const QStringList &blocks = DDiskManager::blockDevices({});
    static QString strblock;
    if (!blocks.isEmpty())
        strblock = blocks.first();
    else
        strblock = "/org/freedesktop/UDisks123";
    // 打桩满足条件
    QStringList(*stu_diskDevices)() = []()->QStringList{
        return QStringList({"/org/freedesktop/UDisks2/drives/AAAA"});
    };
    Stub stu1;
    QStringList(DDiskManager::*m_diskDevices)()const = &DDiskManager::diskDevices;
    stu1.set(m_diskDevices, stu_diskDevices);

    QStringList(*stu_mediaCompatibility)() = []()->QStringList{
            return QStringList({"optical"});
    };
    Stub stu2;
    stu2.set(ADDR(DDiskDevice, mediaCompatibility), stu_mediaCompatibility);

    QString(*stu_connectionBus)() = []()->QString{
        return "usb";
    };
    Stub stu3;
    stu3.set(ADDR(DDiskDevice, connectionBus), stu_connectionBus);

    QStringList(*stu_blockDevices)(QVariantMap) = [](QVariantMap)->QStringList{
        return QStringList({strblock});
    };
    Stub stu4;
    QStringList (*m_blockDevices)(QVariantMap) = &DDiskManager::blockDevices;
    stu4.set(m_blockDevices, stu_blockDevices);

    QString(*stu_drive)() = []()->QString{
        return "/org/freedesktop/UDisks2/drives/AAAA";
    };
    Stub stu5;
    stu5.set(ADDR(DBlockDevice, drive), stu_drive);

    QByteArrayList(*stu_mountPoints)() = []()->QByteArrayList{
        return QByteArrayList({"/123"});
    };
    Stub stu6;
    stu6.set(ADDR(DBlockDevice, mountPoints), stu_mountPoints);

    int mode = 0;
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedOptical(mode));
}

TEST_F(TestAccessControlManager, testChangeMountedOnInit_Block)
{
    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(0x01, QPair<QString, int>("/usr/bin/python3.7", 0));
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedOnInit());
}

TEST_F(TestAccessControlManager, testChangeMountedOnInit_Optical)
{
    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(0x02, QPair<QString, int>("/usr/bin/python3.7", 0));
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedOnInit());
}

TEST_F(TestAccessControlManager, testChangeMountedOnInit_Protocol)
{
    mng->m_globalPolicies.clear();
    mng->m_globalPolicies.insert(0x04, QPair<QString, int>("/usr/bin/python3.7", 0));
    EXPECT_NO_FATAL_FAILURE(mng->changeMountedOnInit());
}

TEST_F(TestAccessControlManager, testAccessMode_Disable)
{
    EXPECT_EQ(mng->accessMode(""), 0);
}

TEST_F(TestAccessControlManager, testAccessMode_Disable2)
{
    long t = time(nullptr);
    QString dirName = QString("mountPoint_W_%1").arg(t);
    QString strDirPath = QDir::separator() + dirName;
    EXPECT_EQ(mng->accessMode(strDirPath), 0);
}

TEST_F(TestAccessControlManager, testAccessMode_R)
{
    EXPECT_EQ(mng->accessMode("/media/root"), 1);
}

TEST_F(TestAccessControlManager, testAccessMode_W)
{
    // 创建一个可以写的挂载点
    QString strHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    long t = time(nullptr);
    QString dirName = QString("mountPoint_W_%1").arg(t);
    QString strDirPath = strHome + QDir::separator() + dirName;
    QDir dir(strDirPath);
    if (dir.mkdir(strDirPath)) {
        if (QFile(strDirPath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
            EXPECT_EQ(mng->accessMode(strDirPath), 2);
    }

    // 删除挂载点
    if (dir.exists()) {
        dir.rmdir(strDirPath);
    }
}

TEST_F(TestAccessControlManager, testSavePolicy1)
{
    QVariantMap policy;
    policy.insert("global", 1);
    policy.insert("invoker", "/usr/bin/python3.7");
    policy.insert("policy", 0);
    policy.insert("timestamp", "1621240815");
    policy.insert("type", 1);
    EXPECT_NO_FATAL_FAILURE(mng->savePolicy(policy));
}

TEST_F(TestAccessControlManager, testSavePolicy2)
{
    QVariantMap policy;
    policy.insert("global", 1);
    policy.insert("invoker", "/usr/bin/python3.7");
    policy.insert("policy", 0);
    policy.insert("timestamp", "1621240815");
    policy.insert("type", 1);
    // 普通用户运行时，并不会创建权限文件，导致代码无法覆盖
    // 所以为了达到覆盖率，普通用户时，做一些特殊处理
    if (getuid() == 0) {
        EXPECT_NO_FATAL_FAILURE(mng->savePolicy(policy));
    } else {
        // 重新设置权限文件路径，普通用户就具有创建文件权限
        QString strHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        long t = time(nullptr);
        QString dirName = QString("devAccessConfig_%1.json").arg(t);
        QString strFilePath = strHome + QDir::separator() + dirName;
        QFile file(strFilePath);
        if (file.exists())
            file.remove();
        QString strTmpPath = mng->m_configPath;
        mng->m_configPath = strFilePath;
        EXPECT_NO_FATAL_FAILURE(mng->savePolicy(policy));
        // 再次测试文件存在的情况
        EXPECT_NO_FATAL_FAILURE(mng->savePolicy(policy));
        // 还原路径，删除文件
        mng->m_configPath = strTmpPath;
        file.remove();
    }
}

TEST_F(TestAccessControlManager, testLoadPolicy)
{
    // 普通用户运行时，如果权限文件不存在，无法创建权限文件，导致代码无法覆盖
    // 所以为了达到覆盖率，普通用户时，做一些特殊处理
    if (getuid() == 0) {
        EXPECT_NO_FATAL_FAILURE(mng->loadPolicy());
    } else {
        // 普通目录下，创建权限文件
        QVariantMap policy;
        policy.insert("global", 1);
        policy.insert("invoker", "/usr/bin/python3.7");
        policy.insert("policy", 0);
        policy.insert("timestamp", "1621240815");
        policy.insert("type", 1);
        QString strHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        long t = time(nullptr);
        QString dirName = QString("devAccessConfig_%1.json").arg(t);
        QString strFilePath = strHome + QDir::separator() + dirName;
        QFile file(strFilePath);
        if (file.exists())
            file.remove();
        QString strTmpPath = mng->m_configPath;
        mng->m_configPath = strFilePath;
        mng->savePolicy(policy);

        EXPECT_NO_FATAL_FAILURE(mng->loadPolicy());

        // 还原路径，删除文件
        mng->m_configPath = strTmpPath;
        file.remove();
    }
}

TEST_F(TestAccessControlManager, testDecodeConfig)
{
    EXPECT_NO_FATAL_FAILURE(mng->decodeConfig());
}

TEST_F(TestAccessControlManager, testEncodeConfig)
{
    EXPECT_NO_FATAL_FAILURE(mng->encodeConfig());
}
