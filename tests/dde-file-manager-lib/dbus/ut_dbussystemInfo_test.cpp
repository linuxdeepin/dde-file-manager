#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include <QProcess>

#define private public
#define protected public

#include "dbus/dbussysteminfo.h"
#include "stub.h"

using namespace testing;
namespace  {
    class DBusSystemInfoTest : public Test
    {
    public:
        DBusSystemInfoTest():Test()
        {
        }

        virtual void SetUp() override {
            p_info = new DBusSystemInfo();
        }

        virtual void TearDown() override {
            delete p_info;
        }

        DBusSystemInfo * p_info;
    };
}

TEST_F(DBusSystemInfoTest, get_system_type)
{
    qint64 type = p_info->systemType();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.SystemType");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(QString::number(type)));
}

TEST_F(DBusSystemInfoTest, get_distro_desc)
{
    QString distroDesc = p_info->distroDesc();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.DistroDesc");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(distroDesc));
}

TEST_F(DBusSystemInfoTest, get_distro_ID)
{
    QString distroID = p_info->distroID();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.DistroID");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(distroID));
}

TEST_F(DBusSystemInfoTest, get_distro_ver)
{
    QString distroVer = p_info->distroVer();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.DistroVer");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(distroVer));
}

TEST_F(DBusSystemInfoTest, get_processor)
{
    QString processor = p_info->processor();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.Processor");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(processor));
}

TEST_F(DBusSystemInfoTest, get_version)
{
    QString version = p_info->version();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.Version");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(version));
}

TEST_F(DBusSystemInfoTest, get_disk_cap)
{
    quint64 diskCap = p_info->diskCap();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.DiskCap");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(QString::number(diskCap)));
}

TEST_F(DBusSystemInfoTest, get_memory_cap)
{
    quint64 memoryCap = p_info->memoryCap();
    QProcess process;
    process.start("qdbus --literal com.deepin.daemon.SystemInfo /com/deepin/daemon/SystemInfo com.deepin.daemon.SystemInfo.MemoryCap");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString outputstr(output);
    EXPECT_TRUE(outputstr.contains(QString::number(memoryCap)));
}

TEST_F(DBusSystemInfoTest, tst_property_changed)
{
    QString interfaceName("com.deepin.daemon.SystemInfo");

    QVariantMap changedProps;
    changedProps.insert(QString("Version"), QString("test"));
    QDBusArgument dbusArgument;
    dbusArgument << changedProps;
    QVariant arg2 = QVariant::fromValue(dbusArgument);

    QList<QVariant> arguments;
    arguments << QVariant(interfaceName) << arg2 << QVariant("test");

    QDBusMessage msg;
    msg.setArguments(arguments);

    p_info->__propertyChanged__(msg);
}
