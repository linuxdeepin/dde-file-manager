#include <gtest/gtest.h>
#include <dsysinfo.h>
#include "stub.h"

#include "dfmglobal.h"
#include "dbus/dbussysteminfo.h"

#include "dialogs/computerpropertydialog.h"

DCORE_USE_NAMESPACE

namespace  {
    class TestComputerPropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new ComputerPropertyDialog();
            std::cout << "start TestComputerPropertyDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestComputerPropertyDialog";
        }
    public:
        ComputerPropertyDialog *m_pTester;
    };
}

TEST_F(TestComputerPropertyDialog, testInit)
{
    EXPECT_NE(nullptr, m_pTester);
}

TEST_F(TestComputerPropertyDialog, testInitUI)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };

    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    m_pTester->initUI();
    EXPECT_EQ(420, m_pTester->height());
}

TEST_F(TestComputerPropertyDialog, testInitUI2)
{
    QString(*stub_distributionOrgLogo)(DSysInfo::OrgType, DSysInfo::LogoType, const QString &) = [](DSysInfo::OrgType orgType, DSysInfo::LogoType type, const QString &fallback )->QString{
        Q_UNUSED(orgType);
        Q_UNUSED(type);
        Q_UNUSED(fallback);
        return "";
    };

    Stub stu;
    stu.set(ADDR(DSysInfo, distributionOrgLogo), stub_distributionOrgLogo);

    m_pTester->initUI();
    EXPECT_EQ(320, m_pTester->width());
}

TEST_F(TestComputerPropertyDialog, testGetMessage)
{
    QHash<QString, QString> datas;
    QStringList data;
    data << "计算机名：" << "版本：" << "系统类型：" << "处理器：" << "内存：" << "UOS";
    datas = m_pTester->getMessage(data);
    EXPECT_GT(datas.size(), 0);
}

TEST_F(TestComputerPropertyDialog, testGetMessage2)
{
    bool(*stub_isValid)() = []()->bool{
        return false;
    };
    Stub stu2;
    stu2.set(ADDR(DBusSystemInfo, isValid), stub_isValid);

    QHash<QString, QString> datas;
    QStringList data;
    data << "计算机名：" << "版本：" << "系统类型：" << "处理器：" << "内存：" << "UOS";
    datas = m_pTester->getMessage(data);
    EXPECT_GT(datas.size(), 0);
}

TEST_F(TestComputerPropertyDialog, testGetMessage3)
{
    QString(*stub_version)() = []()->QString{
        return "professional";
    };
    Stub stu;
    stu.set(ADDR(DBusSystemInfo, version), stub_version);

    QHash<QString, QString> datas;
    QStringList data;
    data << "计算机名：" << "版本：" << "系统类型：" << "处理器：" << "内存：" << "UOS";
    datas = m_pTester->getMessage(data);
    EXPECT_GT(datas.size(), 0);
}
