#include "dialogs/computerpropertydialog.h"

#include <gtest/gtest.h>

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
    m_pTester->show();
}

TEST_F(TestComputerPropertyDialog, testGetMessage)
{
//    QHash<QString, QString> datas;
//    QStringList data;
//    data << "计算机名：" << "版本：" << "系统类型：" << "处理器：" << "内存：";
//    datas = m_pTester->getMessage(data);
//    EXPECT_GT(datas.size(), 0);
}
