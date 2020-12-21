#include <gtest/gtest.h>
#include "stub.h"

#include "desktopinfo.h"
#include "dfmglobal.h"

#define private public
#include "dialogs/burnoptdialog.h"

namespace  {
    class TestBurnOptDialog : public testing::Test{
    public:
        void SetUp() override
        {
            m_pTester = new BurnOptDialog("file:///file");
            std::cout << "start TestBurnOptDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestBurnOptDialog";
        }
    public:
        BurnOptDialog *m_pTester;
    };
}

TEST_F(TestBurnOptDialog, testInit)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);
    BurnOptDialog dlg("file:///file");
    EXPECT_NE(nullptr, m_pTester);
}

TEST_F(TestBurnOptDialog, testInit2)
{
    emit m_pTester->buttonClicked(0, "");
}

TEST_F(TestBurnOptDialog, testInit3)
{
    emit m_pTester->buttonClicked(1, "");
}

TEST_F(TestBurnOptDialog, testSetIOSImage)
{
    DUrl image("file:///test1/image");
    m_pTester->setISOImage(image);
    QString str = m_pTester->d_ptr->image_file.toString();
    EXPECT_TRUE(str == "file:///test1/image");
}

TEST_F(TestBurnOptDialog, testSetJobWindowId)
{
    int wid(12345678);
    m_pTester->setJobWindowId(wid);
    EXPECT_EQ(wid, m_pTester->d_ptr->window_id);
}

TEST_F(TestBurnOptDialog, testSetDefaultVolName)
{
    bool(*stub_waylandDectected)() = []()->bool{
            return true;
    };

    Stub stu;
    stu.set(ADDR(DesktopInfo, waylandDectected), stub_waylandDectected);

    QString volName("test UT");
    m_pTester->setDefaultVolName(volName);
    QString str = m_pTester->d_ptr->le_volname->text();
    EXPECT_TRUE(str == "test UT");
}

