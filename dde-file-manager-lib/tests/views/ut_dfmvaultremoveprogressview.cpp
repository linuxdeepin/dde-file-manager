#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <DWaterProgress>
#include <qprocess.h>
#include <interfaces/dfmstandardpaths.h>

#define private public
#include "views/dfmvaultremoveprogressview.h"

DWIDGET_USE_NAMESPACE
namespace  {
    class TestDFMVaultRemoveProgressView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultRemoveProgressView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultRemoveProgressView>(new DFMVaultRemoveProgressView());
            m_view->show();
            std::cout << "start TestDFMVaultRemoveProgressView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultRemoveProgressView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultRemoveProgressView, tst_clear)
{
    EXPECT_EQ(static_cast<int>(0), m_view->m_vaultRmProgressBar->value());
    EXPECT_EQ(static_cast<int>(0), m_view->m_iFiles);
    EXPECT_EQ(static_cast<int>(0), m_view->m_iRmFiles);
    EXPECT_EQ(static_cast<int>(0), m_view->m_iRmDir);
}

TEST_F(TestDFMVaultRemoveProgressView, tst_statisticsFiles)
{
    QString homePath = DFMStandardPaths::location(DFMStandardPaths::HomePath);
    QString path = homePath + "/TestDFMVaultRemoveProgressView/";
    QProcess::execute("mkdir " + path);

    QProcess::execute("touch " + path + "test.txt");
    QProcess::execute("mkdir " + path + "child");
    QProcess::execute("touch " + path + "child/" "test.txt");

    m_view->statisticsFiles(path);

    EXPECT_EQ(3, m_view->m_iFiles);

    QProcess::execute("rm -r " + path);
}


TEST_F(TestDFMVaultRemoveProgressView, tst_removeFile)
{
    QString homePath = DFMStandardPaths::location(DFMStandardPaths::HomePath);
    QString path = homePath + "/TestDFMVaultRemoveProgressView/";
    QProcess::execute("mkdir " + path);

    QProcess::execute("touch " + path + "test.txt");
    QProcess::execute("mkdir " + path + "child");
    QProcess::execute("touch " + path + "child/" "test.txt");

    m_view->statisticsFiles(path);
    m_view->removeFileInDir(path + "child");

    EXPECT_EQ(1, m_view->m_iRmFiles);

    QProcess::execute("rm -r " + path);
}

TEST_F(TestDFMVaultRemoveProgressView, tst_onFileRemove)
{
    m_view->onFileRemove(10);
    EXPECT_EQ(10, m_view->m_vaultRmProgressBar->value());
}
